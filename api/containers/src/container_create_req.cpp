#include "container_create_req.h"
#include <array>
#include <expected>
#include <ranges>
#include <string>
#include <simdjson.h>

namespace nanoenv::api::containers {
    std::expected<ContainerCreateReq, std::string> ContainerCreateReq::fromRequest(
        const drogon::HttpRequest &req
    ) {
        simdjson::ondemand::parser jsonParser;
        const std::string_view body = req.body();
        auto doc = jsonParser.iterate(body.data(), body.size());

        // CPU
        auto cpuResult = doc["cpu"].get_int64();
        int64_t cpu =
            (cpuResult.error() == simdjson::error_code::NO_SUCH_FIELD) ? defaultCpu : cpuResult.value_unsafe();

        if (cpu < 1 || cpu > 16) {
            return std::unexpected("CPU must be between 1 and 16.");
        }

        // Memory
        auto memoryResult = doc["memory"].get_int64();
        int64_t memory =
            (memoryResult.error() == simdjson::error_code::NO_SUCH_FIELD) ? defaultMemory : memoryResult.value_unsafe();

        if (memory < 512 || memory > 64000) {
            return std::unexpected("Memory must be between 512MB and 64GB.");
        }

        // Name
        auto name_result = doc["name"].get_string();
        if (name_result.error() == simdjson::error_code::NO_SUCH_FIELD) {
            return std::unexpected("Missing required parameter: 'name'.");
        }

        std::string_view name = name_result.value_unsafe();
        if (name.empty()) {
            return std::unexpected("Parameter 'name' must not be empty.");
        }

        // Image
        auto imageResult = doc["image"].get_string();
        if (imageResult.error() == simdjson::error_code::NO_SUCH_FIELD) {
            return std::unexpected("Missing required parameter: 'image'.");
        }

        std::string_view image = imageResult.value_unsafe();
        if (image.empty()) {
            return std::unexpected("Parameter 'image' must not be empty.");
        }

        // Network extraction
        auto network_result = doc["network"].get_string();
        std::string_view network = (network_result.error() == simdjson::error_code::NO_SUCH_FIELD)
            ? defaultNetwork
            : network_result.value_unsafe();

        if (!std::ranges::contains(validNetworks, network)) {
            return std::unexpected("Network must be 'bridge', 'host', or 'none'.");
        }

        // Construct the request object
        ContainerCreateReq request{
            std::string(name), std::string(image), static_cast<int>(cpu), static_cast<int>(memory), std::string(network)
        };

        return {std::move(request)};
    }
};// namespace nanoenv::api::containers
