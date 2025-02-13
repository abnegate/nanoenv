#include "oci_image.h"
#include "thread_pool.h"
#include <filesystem>
#include <format>
#include <fstream>
#include <future>
#include <mutex>
#include <string>
#include <thread>
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

        threads::ThreadPool &pool = getThreadPool();
        std::vector<std::future<std::expected<bool, std::string>>> workers;
        for (const auto &digest : *layerDigestsRes) {
            workers.push_back(pool.enqueue([registry, image, outputDir, digest]() -> std::expected<bool, std::string> {
                return downloadLayer(registry, image, digest, outputDir);
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

    std::expected<bool, std::string> OCIImage::downloadLayer(
        const std::string &registry,
        const std::string &image,
        const std::string &digest,
        const std::string &outputDir
    ) noexcept {
        int pipefd[2];
        if (pipe(pipefd) < 0) {
            return std::unexpected("Failed to create pipe.");
        }

        // Use the thread pool to launch the download task.
        threads::ThreadPool &pool = getThreadPool();
        auto downloadFuture = pool.enqueue([=]() -> std::expected<bool, std::string> {
            thread_local CURL *curl = nullptr;
            if (!curl) {
                curl = curl_easy_init();
                if (!curl) {
                    return std::unexpected("Failed to initialize cURL in download task.");
                }
            }
            curl_easy_reset(curl);

            const std::string url = std::format("https://{}/v2/{}/blobs/{}", registry, image, digest);

            curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE);
            curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
            curl_easy_setopt(curl, CURLOPT_PIPEWAIT, 1L);
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &pipefd[1]);
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
            close(pipefd[1]);

            if (res != CURLE_OK) {
                return std::unexpected("Failed to download layer: " + std::string(curl_easy_strerror(res)));
            }

            return true;
        });

        // In the current thread, use libarchive to read from the pipe.
        archive *a = archive_read_new();
        if (!a) {
            close(pipefd[0]);
            return std::unexpected("Failed to create archive object.");
        }

        archive_read_support_format_tar(a);
        archive_read_support_filter_all(a);

        constexpr size_t bufSize = 64 * 1024;
        if (archive_read_open_fd(a, pipefd[0], bufSize) != ARCHIVE_OK) {
            archive_read_free(a);
            close(pipefd[0]);
            return std::unexpected("Failed to open archive from pipe.");
        }

        // Offload file writes using the thread pool.
        std::vector<std::future<std::expected<bool, std::string>>> writeFutures;

        archive_entry *entry;

        while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
            std::string filePath = outputDir + "/" + archive_entry_pathname(entry);
            std::error_code ec;
            create_directories(std::filesystem::path(filePath).parent_path(), ec);
            mode_t fileMode = archive_entry_perm(entry);

            // Read the file data from the archive.
            std::vector<char> fileData;
            std::vector<char> buffer(bufSize);
            while (true) {
                const ssize_t bytesRead = archive_read_data(a, buffer.data(), buffer.size());
                if (bytesRead > 0) {
                    fileData.insert(fileData.end(), buffer.begin(), buffer.begin() + bytesRead);
                } else if (bytesRead == 0) {
                    break;
                } else {
                    archive_read_close(a);
                    archive_read_free(a);
                    return std::unexpected("Error reading archive entry: " + filePath);
                }
            }

            // Offload file write task.
            writeFutures.push_back(pool.enqueue([filePath, fileData = std::move(fileData), fileMode]() mutable -> std::expected<bool, std::string> {
#if defined(__linux__)
                    // Linux: Use io_uring for asynchronous file writes.
                    struct io_uring ring;
                    if (io_uring_queue_init(64, &ring, 0) < 0) {
                        return std::unexpected("io_uring_queue_init failed");
                    }
                    int fd = open(filePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, fileMode);
                    if (fd < 0) {
                        io_uring_queue_exit(&ring);
                        return std::unexpected("Failed to open file " + filePath);
                    }
                    size_t totalWritten = 0;
                    while (totalWritten < fileData.size()) {
                        struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
                        if (!sqe) {
                            close(fd);
                            io_uring_queue_exit(&ring);
                            return std::unexpected("Failed to get io_uring SQE");
                        }
                        size_t toWrite = fileData.size() - totalWritten;
                        io_uring_prep_write(sqe, fd, fileData.data() + totalWritten, toWrite, totalWritten);
                        io_uring_submit(&ring);
                        struct io_uring_cqe *cqe;
                        if (io_uring_wait_cqe(&ring, &cqe) < 0) {
                            close(fd);
                            io_uring_queue_exit(&ring);
                            return std::unexpected("io_uring_wait_cqe failed");
                        }
                        if (cqe->res < 0) {
                            close(fd);
                            io_uring_cqe_seen(&ring, cqe);
                            io_uring_queue_exit(&ring);
                            return std::unexpected("io_uring write error: " + std::to_string(cqe->res));
                        }
                        totalWritten += cqe->res;
                        io_uring_cqe_seen(&ring, cqe);
                    }
                    close(fd);
                    io_uring_queue_exit(&ring);
                    return true;
#elif defined(__APPLE__)
                    // macOS: Use POSIX AIO and kqueue to wait for asynchronous writes.
                    int fd = open(filePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, fileMode);
                    if (fd < 0) {
                        return std::unexpected("Failed to open file " + filePath);
                    }

                    size_t totalWritten = 0;
                    const int kq = kqueue();
                    if (kq == -1) {
                        close(fd);
                        return std::unexpected("Failed to create kqueue");
                    }

                    while (totalWritten < fileData.size()) {
                        aiocb cb = {};
                        cb.aio_fildes = fd;
                        cb.aio_buf = fileData.data() + totalWritten;
                        size_t toWrite = fileData.size() - totalWritten;
                        cb.aio_nbytes = toWrite;
                        cb.aio_offset = totalWritten;
                        if (aio_write(&cb) < 0) {
                            close(fd);
                            close(kq);
                            return std::unexpected("aio_write failed");
                        }

                        // Wait for AIO event via kqueue.
                        struct kevent event{};
                        const int nev = kevent(kq, nullptr, 0, &event, 1, nullptr);
                        if (nev < 0) {
                            close(fd);
                            close(kq);
                            return std::unexpected("kevent wait failed");
                        }

                        const int err = aio_error(&cb);
                        if (err != 0) {
                            close(fd);
                            close(kq);
                            return std::unexpected("aio_write error: " + std::to_string(err));
                        }

                        const ssize_t ret = aio_return(&cb);
                        if (ret < 0) {
                            close(fd);
                            close(kq);
                            return std::unexpected("aio_return error");
                        }

                        totalWritten += ret;
                    }

                    close(fd);
                    close(kq);

                    return true;
#else
                    // Fallback synchronous write.
                    int fd = open(filePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, fileMode);
                    if (fd < 0) {
                        return std::unexpected("Failed to open file " + filePath);
                    }
                    size_t totalWritten = 0;
                    while (totalWritten < fileData.size()) {
                        ssize_t written = write(fd, fileData.data() + totalWritten, fileData.size() - totalWritten);
                        if (written <= 0) {
                            close(fd);
                            return std::unexpected("Write error on file " + filePath);
                        }
                        totalWritten += written;
                    }
                    close(fd);
                    return true;
#endif
                }
            ));
        }
        archive_read_close(a);
        archive_read_free(a);

        // Wait for the download task to finish.
        auto downloadRes = downloadFuture.get();
        if (!downloadRes) {
            return std::unexpected(downloadRes.error());
        }
        // Wait for all file writes to complete.
        for (auto &fut : writeFutures) {
            auto res = fut.get();
            if (!res) {
                return std::unexpected(res.error());
            }
        }
        return true;
    }

    // A dedicated thread pool for all tasks; here we use twice the number of CPU cores.
    threads::ThreadPool &OCIImage::getThreadPool() {
        static threads::ThreadPool threadPool(std::thread::hardware_concurrency() * 2);
        return threadPool;
    }

    // Basic image format validation.
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
