#pragma once

#include "thread_pool.h"
#include <expected>
#include <string>
#include <vector>

namespace nanoenv::containers {
    class OCIImage {
    public:
        static std::expected<bool, std::string> processImage(
            const std::string &registry,
            const std::string &image,
            const std::string &outputDir
        ) noexcept;

    private:
        static threads::ThreadPool &getDownloadThreadPool();
        static threads::ThreadPool &getExtractThreadPool();

        static std::string getLayerCachePath(
            const std::string &outputDir,
            const std::string &digest
        );

        static std::expected<bool, std::string> isValidImageFormat(
            const std::string &imag
        ) noexcept;

        static std::expected<std::string, std::string> downloadManifest(
            const std::string &registry,
            const std::string &image
        ) noexcept;

        static std::expected<std::vector<std::string>, std::string> parseLayerDigests(
            const std::string &manifest
        ) noexcept;

        static std::expected<bool, std::string> downloadAndExtractLayer(
            const std::string &registry,
            const std::string &image,
            const std::string &digest,
            const std::string &outputDir
        ) noexcept;

        static std::expected<bool, std::string> extractLayerFromFile(
            const std::string &cachePath,
            const std::string &outputDir
        ) noexcept;
    };
} // namespace nanoenv::containers
