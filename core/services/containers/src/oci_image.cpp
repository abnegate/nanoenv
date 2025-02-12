#include "oci_image.h"
#include "system.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>
#include <archive.h>
#include <archive_entry.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace nanoenv::containers {

    [[nodiscard]] bool OCIImage::isValidImage(
        const std::string &image
    ) noexcept {
        if (!isValidImageFormat(image)) {
            return false;
        }

        if (localImageCache.contains(image)) {
            return true;
        }

        const auto imagePath = std::filesystem::path(OCI_CACHE_DIR) / image;
        if (is_directory(imagePath) || exists(imagePath)) {
            localImageCache.insert(image);
            return true;
        }

        return false;
    }

    bool OCIImage::extract(
        const std::string &imagePath,
        const std::string &containerPath
    ) {
        const int fd = open(imagePath.c_str(), O_RDONLY);
        if (fd < 0) {
            throw std::runtime_error(std::format("Failed to open OCI image '{}'", imagePath));
        }

        struct stat fileStat{};
        if (fstat(fd, &fileStat) < 0) {
            close(fd);
            throw std::runtime_error(std::format("Failed to get file size for '{}'", imagePath));
        }

        // Memory-map the image file (zero-copy read)
        void *mappedData = mmap(nullptr, fileStat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (mappedData == MAP_FAILED) {
            close(fd);
            throw std::runtime_error(std::format("Failed to mmap OCI image '{}'", imagePath));
        }

        // Optimize mmap performance
        madvise(mappedData, fileStat.st_size, MADV_SEQUENTIAL);
        madvise(mappedData, fileStat.st_size, MADV_WILLNEED); // Prefetch pages

#if defined(__linux__)
        if (fileStat.st_size < (100 * 1024 * 1024)) {
            readahead(fd, 0, fileStat.st_size); // Force prefetch into RAM
        } else {
            posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL); // Let kernel handle it
        }
#elif defined(__APPLE__)
        fcntl(fd, F_RDADVISE, nullptr);
#endif

        close(fd);

        // Set up libarchive
        struct archive *archivePtr = archive_read_new();
        archive_read_support_format_tar(archivePtr);
        archive_read_support_filter_all(archivePtr); // Auto-detect compression

        if (archive_read_open_memory(archivePtr, mappedData, fileStat.st_size) != ARCHIVE_OK) {
            munmap(mappedData, fileStat.st_size);
            throw std::runtime_error("Failed to open archive from memory.");
        }

        // Queue for file extraction tasks
        std::queue<std::pair<std::string, archive_entry *>> taskQueue;
        std::mutex queueMutex;
        std::condition_variable queueCond;

        struct archive_entry *entryPtr;
        while (archive_read_next_header(archivePtr, &entryPtr) == ARCHIVE_OK) {
            {
                std::string entryPath = std::format("{}/{}", containerPath, archive_entry_pathname(entryPtr));
                std::lock_guard lock(queueMutex);
                taskQueue.emplace(
                    std::move(entryPath), archive_entry_clone(entryPtr)
                ); // Clone entry for multi-threading
            }
        }
        queueCond.notify_all(); // Notify all at once

        // Worker pool for parallel extraction
        const size_t numThreads = std::thread::hardware_concurrency();
        std::vector<std::thread> workers;

        auto workerFunc = [&] {
            constexpr size_t bufferSize = 64 * 1024;
            void *buffer;
            posix_memalign(&buffer, 4096, bufferSize);

            while (true) {
                std::pair<std::string, archive_entry *> task;
                {
                    std::unique_lock lock(queueMutex);
                    if (taskQueue.empty()) {
                        free(buffer);
                        return;
                    }
                    task = std::move(taskQueue.front());
                    taskQueue.pop();
                }

                const std::string &entryPath = task.first;
                archive_entry *workerEntry = task.second;

                // Open file with optimized IO flags
#if defined(__linux__)
                int outFd =
                    open(entryPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_DIRECT, archive_entry_perm(workerEntry));
#elif defined(__APPLE__)
                int outFd = open(entryPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, archive_entry_perm(workerEntry));
                fcntl(outFd, F_NOCACHE, 1);
#endif

                if (outFd < 0) {
                    std::cerr << "Failed to create '" << entryPath << "'\n";
                    archive_entry_free(workerEntry);
                    continue;
                }

                ssize_t bytesRead;
                while ((bytesRead = archive_read_data(archivePtr, buffer, bufferSize)) > 0) {
                    write(outFd, buffer, bytesRead); // Unbuffered direct write
                }

                close(outFd);
                archive_entry_free(workerEntry);
            }
        };

        // Start worker threads
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back(workerFunc);
        }

        // Wait for workers to finish
        for (auto &worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }

        // Cleanup
        archive_read_close(archivePtr);
        archive_read_free(archivePtr);
        munmap(mappedData, fileStat.st_size);
    }

    [[nodiscard]] bool OCIImage::isValidImageFormat(
        const std::string &image
    ) {
        if (image.empty() || image.length() > 256) {
            return false; // Too long or empty
        }

        bool hasRepo = false;
        bool hasTag = false;
        bool hasDigest = false;
        bool hasInvalid = false;
        size_t lastSlash = 0;
        size_t lastColon = 0;
        size_t lastAt = 0;

        for (size_t i = 0; i < image.size(); i++) {
            const char character = image[i];

            if (character == '/') {
                hasRepo = true;
                lastSlash = i;
            } else if (character == ':') {
                if (hasTag) {
                    return false; // Multiple `:` → invalid
                }
                hasTag = true;
                lastColon = i;
            } else if (character == '@') {
                if (hasDigest) {
                    return false; // Multiple `@` → invalid
                }
                hasDigest = true;
                lastAt = i;
            } else if (!(std::isalnum(character) || character == '.' || character == '_' || character == '-')) {
                hasInvalid = true;
                break;
            }
        }

        if (hasInvalid) {
            return false;
        }

        // If ':' or '@' are present, ensure they aren't at the start/end
        if ((hasTag && (lastColon == 0 || lastColon == image.size() - 1)) ||
            (hasDigest && (lastAt == 0 || lastAt == image.size() - 1))) {
            return false;
        }

        // Validate digest (if present)
        if (hasDigest) {
            const std::string_view digest = image.substr(lastAt + 1);

            // Manually check "sha256:<64-hex-digits>"
            if (digest.size() != 71 || digest.substr(0, 7) != "sha256:") {
                return false;
            }
            for (size_t i = 7; i < 71; i++) {
                if (!std::isxdigit(digest[i])) {
                    return false;
                }
            }
        }

        // Allow `alpine`, `repo/image:tag`, or `repo/image@sha256:hash`
        return true;
    }
} // namespace nanoenv::containers
