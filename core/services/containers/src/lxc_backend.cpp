#include "lxc_backend.h"
#include "system.h"
#include <fstream>
#include <iostream>
#include <sys/stat.h>

namespace nanoenv::containers {
    bool LXCBackend::createContainerImpl(
        const std::string &name,
        const std::string &image,
        int cpu,
        int memory,
        const std::string &network
    ) {
        // // Validate that the image exists
        // if (!isValidOciImage(imagePath)) {
        //     throw std::runtime_error(std::format("Image '{}' not found or invalid.", imagePath));
        // }
        //
        // // Generate a unique container ID
        // const std::string containerId = system::System::generateUUID();
        //
        // // Define container root path
        // const std::string containerPath = std::format("{}/{}", containerBasePath, containerId);
        //
        // // Create container storage directory
        // if (mkdir(containerPath.c_str(), 0755) != 0) {
        //     throw std::runtime_error(std::format("Failed to create container directory: {}", containerPath));
        // }
        //
        // // Extract OCI Image
        // extractOCIImageImpl(imagePath, containerPath);
        //
        // // Generate LXC Configuration
        // const std::string configPath = std::format("{}/config", containerPath);
        // std::ofstream configFile(configPath);
        // if (!configFile) {
        //     throw std::runtime_error(std::format("Failed to create container config file: {}", configPath));
        // }
        //
        // configFile << std::format(
        //     R"(
        //     lxc.uts.name = {}
        //     lxc.rootfs.path = {}
        //     lxc.network.type = veth
        //     lxc.network.link = br0
        //     lxc.network.flags = up
        //     lxc.start.auto = 1
        //     lxc.start.delay = 0
        //     )",
        //     name,
        //     containerPath
        // );
        //
        // configFile.close();
        //
        // // Start the container using LXC
        // if (!startContainer(containerId)) {
        //     throw std::runtime_error(std::format("Failed to start container '{}'", containerId));
        // }
        //
        // return containerId; // Return the generated container ID
        return true;
    }

    bool LXCBackend::startContainerImpl(
        const std::string &name
    ) {}

    bool LXCBackend::stopContainerImpl(
        const std::string &name
    ) {}

    bool LXCBackend::destroyContainerImpl(
        const std::string &name
    ) {}
} // namespace nanoenv::containers
