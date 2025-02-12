#pragma once

#include <string>
#include <unordered_set>

namespace nanoenv::containers {
    class OCIImage {
    public:
        OCIImage() = default;
        ~OCIImage() = default;

        /**
         * @brief Validates whether the given image string follows the OCI image format.
         *
         * The function checks for valid OCI image naming conventions, including:
         * - Repository names (e.g., `repo/image`)
         * - Tags (e.g., `repo/image:tag`)
         * - Digest hashes (e.g., `repo/image@sha256:hash`)
         *
         * @param image The OCI image string to validate.
         * @return True if the image follows a valid OCI format, false otherwise.
         *
         * @note This function does not perform network checks—only string validation.
         * @warning The function does not check if the image actually exists.
         */
        bool isValidImage(const std::string &image);

        /**
         * @brief Extracts the given OCI image to the specified container path.
         *
         * @param imagePath The OCI image to extract.
         * @param containerPath The path to extract the image to.
         * @return True if the image was successfully extracted, false otherwise.
         */
        bool extract(const std::string &imagePath, const std::string &containerPath);

    private:
        static constexpr auto OCI_CACHE_DIR = "/var/lib/nanoenv/oci_cache/";
        std::unordered_set<std::string> localImageCache;

        static bool isValidImageFormat(const std::string &image);
    };
} // namespace nanoenv::containers
