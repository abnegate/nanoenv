#include "oci_image.h"
#include "thread_pool.h"
#include <filesystem>
#include <format>
#include <fstream>
#include <future>
#include <string>
#include <vector>
#include <archive.h>
#include <archive_entry.h>
#include <curl/curl.h>
#include <fcntl.h>
#include <simdjson.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#if defined(__linux__)
#include <liburing.h> // Linux-specific io_uring
#elif defined(__APPLE__)
#include <aio.h>
#include <sys/event.h>
#endif

namespace nanoenv::containers {

    std::expected<bool, std::string> OCIImage::processImage(
        const std::string &registry,
        const std::string &image,
        const std::string &outputDir
    ) noexcept {
        if (!isValidImageFormat(image)) {
            return std::unexpected("Invalid image format.");
        }

        auto manifestRes = downloadManifest(registry, image);
        if (!manifestRes) {
            return std::unexpected(manifestRes.error());
        }

        auto layerDigestsRes = parseLayerDigests(*manifestRes);
        if (!layerDigestsRes) {
            return std::unexpected(layerDigestsRes.error());
        }

        threads::ThreadPool &pool = getDownloadThreadPool();
        std::vector<std::future<std::expected<bool, std::string>>> workers;
        for (const auto &digest : *layerDigestsRes) {
            workers.push_back(pool.enqueue([registry, image, outputDir, digest]() -> std::expected<bool, std::string> {
                return downloadAndExtractLayer(registry, image, digest, outputDir);
            }));
        }

        for (auto &w : workers) {
            auto res = w.get();
            if (!res) {
                return std::unexpected(res.error());
            }
        }
        return true;
    }

    std::expected<std::string, std::string> OCIImage::downloadManifest(
        const std::string &registry,
        const std::string &image
    ) noexcept {
        thread_local CURL *curl = nullptr;
        if (!curl) {
            curl = curl_easy_init();
            if (!curl) {
                return std::unexpected("Failed to initialize cURL.");
            }
        }
        curl_easy_reset(curl);

        std::string response;

        const auto url = std::format("https://{}/v2/{}/manifests/latest", registry, image);

        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE);
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
        curl_easy_setopt(curl, CURLOPT_PIPEWAIT, 1L);
        curl_easy_setopt(curl, CURLOPT_MAXCONNECTS, 10L);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(
            curl,
            CURLOPT_WRITEFUNCTION,
            +[](void *data, size_t size, size_t nmemb, void *userp) -> size_t {
                auto *str = static_cast<std::string *>(userp);
                str->append(static_cast<char *>(data), size * nmemb);
                return size * nmemb;
            }
        );

        const CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            return std::unexpected("Failed to download manifest: " + std::string(curl_easy_strerror(res)));
        }
        return response;
    }

    std::expected<std::vector<std::string>, std::string> OCIImage::parseLayerDigests(
        const std::string &manifest
    ) noexcept {

        thread_local simdjson::ondemand::parser jsonParser;
        simdjson::ondemand::document doc;

        if (const auto _ = jsonParser.iterate(manifest).get(doc)) {
            return std::unexpected("Failed to parse manifest JSON.");
        }

        std::vector<std::string> digests;

        try {
            for (auto layer : doc["layers"]) {
                digests.emplace_back(layer["digest"].get_string().value_unsafe());
            }
        } catch (...) {
            return std::unexpected("Failed parsing layer digests.");
        }
        return digests;
    }

    std::expected<bool, std::string> OCIImage::extractLayerFromFile(
        const std::string &cachePath,
        const std::string &outputDir
    ) noexcept {
        const int fd = open(cachePath.c_str(), O_RDONLY);
        if (fd < 0) {
            return std::unexpected("Failed to open cached layer file: " + cachePath);
        }

        struct stat st {};
        if (fstat(fd, &st) < 0) {
            close(fd);
            return std::unexpected("Failed to stat cached layer file: " + cachePath);
        }

        archive *a = archive_read_new();

        if (!a) {
            close(fd);
            return std::unexpected("Failed to create archive object.");
        }

        archive_read_support_format_tar(a);
        archive_read_support_filter_all(a);
        archive_read_support_filter_program(a, "pigz");

        if (archive_read_open_fd(a, fd, 64 * 1024) != ARCHIVE_OK) {
            archive_read_free(a);
            close(fd);
            return std::unexpected("Failed to open archive from cached file.");
        }

        archive_entry *entry;
        constexpr size_t bufSize = 64 * 1024;

        while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
            std::string outPath = outputDir + "/" + archive_entry_pathname(entry);

            if (mkdir(std::filesystem::path(outPath).parent_path().c_str(), 0755) != 0 && errno != EEXIST) {
                return std::unexpected("Failed to create directories.");
            }

            const mode_t fileMode = archive_entry_perm(entry);
            const int outFd = open(outPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, fileMode);
            if (outFd < 0) {
                continue;
            }

            void *buf = nullptr;
            if (posix_memalign(&buf, 4096, bufSize) != 0) {
                close(outFd);
                archive_read_close(a);
                archive_read_free(a);
                return std::unexpected("Failed to allocate aligned buffer.");
            }

            ssize_t len = 0;
            while ((len = archive_read_data(a, buf, bufSize)) > 0) {
                const ssize_t written = write(outFd, buf, len);
                if (written != len) {
                    free(buf);
                    close(outFd);
                    archive_read_close(a);
                    archive_read_free(a);
                    return std::unexpected("Failed to write extracted file: " + outPath);
                }
            }

            free(buf);
            close(outFd);
        }

        archive_read_close(a);
        archive_read_free(a);

        return true;
    }

    std::expected<bool, std::string> OCIImage::downloadAndExtractLayer(
        const std::string &registry,
        const std::string &image,
        const std::string &digest,
        const std::string &outputDir
    ) noexcept {
        std::string cachePath = getLayerCachePath(outputDir, digest);
        if (std::filesystem::exists(cachePath)) {
            return extractLayerFromFile(cachePath, outputDir);
        }

        if (mkdir(std::filesystem::path(cachePath).parent_path().c_str(), 0755) != 0 && errno != EEXIST) {
            return std::unexpected("Failed to create directories.");
        }

        // Create three pipes.
        int dp[2];
        int diskPipe[2];
        int extPipe[2];

        if (pipe(dp) < 0) {
            return std::unexpected("Failed to create download pipe.");
        }

        if (pipe(diskPipe) < 0) {
            close(dp[0]);
            close(dp[1]);
            return std::unexpected("Failed to create disk pipe.");
        }

        if (pipe(extPipe) < 0) {
            close(dp[0]);
            close(dp[1]);
            close(diskPipe[0]);
            close(diskPipe[1]);
            return std::unexpected("Failed to create extraction pipe.");
        }

        threads::ThreadPool &downloadPool = getDownloadThreadPool();
        threads::ThreadPool &extractPool = getExtractThreadPool();

        // 1. Download task: download the layer and write to diskPipe[1].
        auto downloadFuture = downloadPool.enqueue([=]() -> std::expected<bool, std::string> {
            thread_local CURL *curl = nullptr;
            if (!curl) {
                curl = curl_easy_init();
                if (!curl)
                    return std::unexpected("Failed to initialize cURL in download task.");
            }
            curl_easy_reset(curl);

            const std::string url = std::format("https://{}/v2/{}/blobs/{}", registry, image, digest);

            curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE);
            curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
            curl_easy_setopt(curl, CURLOPT_PIPEWAIT, 1L);
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &dp[1]);
            curl_easy_setopt(
                curl,
                CURLOPT_WRITEFUNCTION,
                +[](void *data, size_t size, size_t nmemb, void *userp) -> size_t {
                    const int *fdPtr = static_cast<int *>(userp);
                    const size_t totalSize = size * nmemb;
                    const ssize_t written = write(*fdPtr, data, totalSize);
                    return (written < 0) ? 0 : static_cast<size_t>(written);
                }
            );

            const CURLcode res = curl_easy_perform(curl);

            close(dp[1]);
            if (res != CURLE_OK) {
                return std::unexpected("Failed to download layer: " + std::string(curl_easy_strerror(res)));
            }

            return true;
        });

        // 2. Multiplexer task: read from dp[0] and tee into diskPipe[1] and extPipe[1].
        auto muxFuture = downloadPool.enqueue([=]() -> std::expected<bool, std::string> {
#if defined(__linux__)
            constexpr size_t bufSize = 64 * 1024;

            // Open /dev/null for draining.
            int devnull = open("/dev/null", O_WRONLY);
            if (devnull < 0)
                return std::unexpected("Failed to open /dev/null");
            while (true) {
                ssize_t n = tee(dp[0], diskPipe[1], bufSize, SPLICE_F_NONBLOCK);
                if (n < 0) {
                    close(devnull);
                    return std::unexpected("tee error on diskPipe");
                }

                if (n == 0) {
                    break;
                }

                ssize_t n2 = tee(dp[0], extPipe[1], n, SPLICE_F_NONBLOCK);
                if (n2 < 0) {
                    close(devnull);
                    return std::unexpected("tee error on extPipe");
                }

                // Drain dp[0] using splice to /dev/null.
                ssize_t drained = splice(dp[0], nullptr, devnull, nullptr, n, SPLICE_F_NONBLOCK);
                if (drained < 0) {
                    close(devnull);
                    return std::unexpected("splice drain error");
                }
            }
            close(devnull);
            close(dp[0]);
            close(diskPipe[1]);
            close(extPipe[1]);
            return true;
#elif defined(__APPLE__)
            constexpr size_t bufSize = 64 * 1024;
            char buf[bufSize];
            ssize_t n;
            while ((n = read(dp[0], buf, bufSize)) > 0) {
                ssize_t w1 = write(diskPipe[1], buf, n);
                ssize_t w2 = write(extPipe[1], buf, n);
                if (w1 != n || w2 != n)
                    return std::unexpected("Multiplexer write error.");
            }
            close(dp[0]);
            close(diskPipe[1]);
            close(extPipe[1]);
            return true;
#else
            constexpr size_t bufSize = 64 * 1024;
            char buf[bufSize];
            ssize_t n;
            while ((n = read(dp[0], buf, bufSize)) > 0) {
                ssize_t w1 = write(diskPipe[1], buf, n);
                ssize_t w2 = write(extPipe[1], buf, n);
                if (w1 != n || w2 != n)
                    return std::unexpected("Multiplexer write error.");
            }
            close(dp[0]);
            close(diskPipe[1]);
            close(extPipe[1]);
            return true;
#endif
        });

        // 3. Disk writer task: read from diskPipe[0] and write to cache file.
        auto diskFuture = downloadPool.enqueue([=]() -> std::expected<bool, std::string> {
#if defined(__linux__)
            int cacheFd = open(cachePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (cacheFd < 0) {
                return std::unexpected("Failed to open cache file: " + cachePath);
            }

            struct io_uring ring;
            if (io_uring_queue_init(64, &ring, 0) < 0) {
                close(cacheFd);
                return std::unexpected("io_uring_queue_init failed");
            }

            constexpr size_t spliceSize = 64 * 1024;

            while (true) {
                struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
                if (!sqe) {
                    io_uring_queue_exit(&ring);
                    close(cacheFd);
                    return std::unexpected("Failed to get io_uring SQE");
                }

                io_uring_prep_splice(
                    sqe, diskPipe[0], nullptr, cacheFd, nullptr, spliceSize, SPLICE_F_MOVE | SPLICE_F_NONBLOCK
                );
                io_uring_submit(&ring);

                struct io_uring_cqe *cqe;
                if (io_uring_wait_cqe(&ring, &cqe) < 0) {
                    io_uring_queue_exit(&ring);
                    close(cacheFd);
                    return std::unexpected("io_uring_wait_cqe failed");
                }

                if (cqe->res == 0) {
                    io_uring_cqe_seen(&ring, cqe);
                    break;
                } else if (cqe->res < 0) {
                    io_uring_cqe_seen(&ring, cqe);
                    io_uring_queue_exit(&ring);
                    close(cacheFd);
                    return std::unexpected("io_uring splice error: " + std::to_string(cqe->res));
                }

                io_uring_cqe_seen(&ring, cqe);
            }

            io_uring_queue_exit(&ring);
            close(cacheFd);

            return true;
#elif defined(__APPLE__)
            // On macOS, use POSIX AIO with kqueue for asynchronous writes.
            int cacheFd = open(cachePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (cacheFd < 0)
                return std::unexpected("Failed to open cache file: " + cachePath);
            constexpr size_t bufSize = 64 * 1024;
            std::vector<char> buffer(bufSize);
            while (true) {
                ssize_t n = read(diskPipe[0], buffer.data(), bufSize);
                if (n < 0) {
                    close(cacheFd);
                    return std::unexpected("Read error in disk pipe.");
                }
                if (n == 0)
                    break;
                size_t offset = 0;
                while (offset < (size_t) n) {
                    struct aiocb cb;
                    memset(&cb, 0, sizeof(cb));
                    cb.aio_fildes = cacheFd;
                    // Casting away const is safe here because buffer is writable.
                    cb.aio_buf = buffer.data() + offset;
                    cb.aio_nbytes = n - offset;
                    // Set the offset to the current file size.
                    cb.aio_offset = lseek(cacheFd, 0, SEEK_CUR);
                    if (aio_write(&cb) < 0) {
                        close(cacheFd);
                        return std::unexpected("aio_write failed");
                    }
                    const int kq = kqueue();
                    if (kq == -1) {
                        close(cacheFd);
                        return std::unexpected("Failed to create kqueue");
                    }
                    struct kevent event;
                    const int nev = kevent(kq, nullptr, 0, &event, 1, nullptr);
                    if (nev < 0) {
                        close(kq);
                        close(cacheFd);
                        return std::unexpected("kevent wait failed");
                    }
                    const int err = aio_error(&cb);
                    if (err != 0) {
                        close(kq);
                        close(cacheFd);
                        return std::unexpected("aio_write error: " + std::to_string(err));
                    }
                    const ssize_t ret = aio_return(&cb);
                    if (ret < 0) {
                        close(kq);
                        close(cacheFd);
                        return std::unexpected("aio_return error");
                    }
                    offset += ret;
                    close(kq);
                }
            }
            close(cacheFd);
            return true;
#else
            // Fallback: buffered synchronous loop.
            int cacheFd = open(cachePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (cacheFd < 0)
                return std::unexpected("Failed to open cache file: " + cachePath);
            constexpr size_t bufSize = 64 * 1024;
            std::vector<char> buf(bufSize);
            while (true) {
                ssize_t n = read(diskPipe[0], buf.data(), bufSize);
                if (n < 0) {
                    close(cacheFd);
                    return std::unexpected("read failed");
                }
                if (n == 0)
                    break;
                ssize_t written = write(cacheFd, buf.data(), n);
                if (written != n) {
                    close(cacheFd);
                    return std::unexpected("write failed");
                }
            }
            close(cacheFd);
            return true;
#endif
        });

        // 4. Extraction task: read from extPipe[0] and feed into libarchive concurrently.
        auto extractFuture = extractPool.enqueue([=]() -> std::expected<bool, std::string> {
            archive *a = archive_read_new();

            if (!a) {
                close(extPipe[0]);
                return std::unexpected("Failed to create archive object.");
            }

            archive_read_support_format_tar(a);
            archive_read_support_filter_all(a);
            archive_read_support_filter_program(a, "pigz");

            constexpr size_t bufSize = 64 * 1024;
            if (archive_read_open_fd(a, extPipe[0], bufSize) != ARCHIVE_OK) {
                archive_read_free(a);
                close(extPipe[0]);
                return std::unexpected("Failed to open archive from extraction pipe.");
            }

            archive_entry *entry;
            while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
                std::string filePath = outputDir + "/" + archive_entry_pathname(entry);

                if (mkdir(std::filesystem::path(filePath).parent_path().c_str(), 0755) != 0 && errno != EEXIST) {
                    return std::unexpected("Failed to create directories.");
                }

                const mode_t fileMode = archive_entry_perm(entry);
                const int outFd = open(filePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, fileMode);
                if (outFd < 0) {
                    continue;
                }

                void *buf = nullptr;
                if (posix_memalign(&buf, 4096, bufSize) != 0) {
                    close(outFd);
                    archive_read_close(a);
                    archive_read_free(a);
                    return std::unexpected("Failed to allocate aligned buffer.");
                }

                ssize_t len;
                while ((len = archive_read_data(a, buf, bufSize)) > 0) {
                    const ssize_t written = write(outFd, buf, len);
                    if (written != len) {
                        free(buf);
                        close(outFd);
                        archive_read_close(a);
                        archive_read_free(a);
                        return std::unexpected("Failed to write extracted file: " + filePath);
                    }
                }

                free(buf);
                close(outFd);
            }

            archive_read_close(a);
            archive_read_free(a);
            close(extPipe[0]);

            return true;
        });

        auto downloadRes = downloadFuture.get();
        auto muxRes = muxFuture.get();
        auto diskRes = diskFuture.get();
        auto extractRes = extractFuture.get();

        if (!downloadRes) {
            return std::unexpected(downloadRes.error());
        }
        if (!muxRes) {
            return std::unexpected(muxRes.error());
        }
        if (!diskRes) {
            return std::unexpected(diskRes.error());
        }
        if (!extractRes) {
            return std::unexpected(extractRes.error());
        }

        return true;
    }

    threads::ThreadPool &OCIImage::getDownloadThreadPool() {
        static threads::ThreadPool pool(std::thread::hardware_concurrency() * 2);
        return pool;
    }

    threads::ThreadPool &OCIImage::getExtractThreadPool() {
        static threads::ThreadPool pool(std::thread::hardware_concurrency());
        return pool;
    }

    std::string OCIImage::getLayerCachePath(
        const std::string &outputDir,
        const std::string &digest
    ) {
        std::string sanitized = digest;
        std::replace(sanitized.begin(), sanitized.end(), ':', '_');
        return std::format("{}/layers/{}.tar.gz", outputDir, sanitized);
    }

    [[nodiscard]] std::expected<bool, std::string> OCIImage::isValidImageFormat(
        const std::string &image
    ) noexcept {
        if (image.empty() || image.length() > 256) {
            return false; // Too long or empty.
        }
        bool hasTag = false;
        bool hasDigest = false;
        bool hasInvalid = false;
        size_t lastColon = 0, lastAt = 0;
        for (size_t i = 0; i < image.size(); i++) {
            const char ch = image[i];
            if (ch == ':') {
                if (hasTag)
                    return false;
                hasTag = true;
                lastColon = i;
            } else if (ch == '@') {
                if (hasDigest)
                    return false;
                hasDigest = true;
                lastAt = i;
            } else if (!(std::isalnum(ch) || ch == '.' || ch == '_' || ch == '-')) {
                hasInvalid = true;
                break;
            }
        }
        if (hasInvalid)
            return false;
        if ((hasTag && (lastColon == 0 || lastColon == image.size() - 1)) ||
            (hasDigest && (lastAt == 0 || lastAt == image.size() - 1))) {
            return false;
        }
        if (hasDigest) {
            const std::string_view digest = image.substr(lastAt + 1);
            if (digest.size() != 71 || digest.substr(0, 7) != "sha256:") {
                return false;
            }
            for (size_t i = 7; i < 71; i++) {
                if (!std::isxdigit(digest[i])) {
                    return false;
                }
            }
        }
        return true;
    }
} // namespace nanoenv::containers
