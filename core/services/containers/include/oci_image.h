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
        static threads::ThreadPool &getThreadPool();

        static std::expected<bool, std::string> isValidImageFormat(const std::string &image) noexcept;

        static std::expected<std::string, std::string> downloadManifest(const std::string &registry, const std::string &image) noexcept;

        static std::expected<std::vector<std::string>, std::string> parseLayerDigests(const std::string &manifest) noexcept;

        static std::expected<bool, std::string> OCIImage::downloadLayer(
            const std::string &registry,
            const std::string &image,
            const std::string &digest,
            const std::string &outputDir
        ) noexcept;
    };
} // namespace nanoenv::containers
