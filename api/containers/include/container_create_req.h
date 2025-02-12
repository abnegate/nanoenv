#pragma once

#include <array>
#include <expected>
#include <string>
#include <string_view>
#include <drogon/HttpRequest.h>

namespace nanoenv::api::containers {

    /**
     * @brief Represents a container creation request.
     */
    struct ContainerCreateReq {
        std::string name;
        std::string image;
        int cpu;
        int memory;
        std::string network;

        static constexpr int defaultCpu = 2;
        static constexpr int defaultMemory = 1024;
        static constexpr std::string_view defaultNetwork = "bridge";
        static constexpr std::array<std::string_view, 3> validNetworks = {"bridge", "host", "none"};

        ContainerCreateReq(
            std::string name,
            std::string image,
            int cpu,
            int memory,
            std::string network
        ) :
            name(std::move(name)),
            image(std::move(image)),
            cpu(cpu),
            memory(memory),
            network(std::move(network)) {}

        /**
         * @brief Parses the given HTTP request into a container creation request.
         *
         * @param req The HTTP request to parse.
         * @return The parsed container creation request, or an error message if parsing failed.
         */
        static std::expected<ContainerCreateReq, std::string> fromRequest(const drogon::HttpRequest &req);
    };

} // namespace nanoenv::api::containers
