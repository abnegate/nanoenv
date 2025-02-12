#pragma once

#include "container_manager.h"
#include <string>

namespace nanoenv::containers {
    class LimaBackend : public ContainerManager<LimaBackend> {
    public:
        bool createContainerImpl(
            const std::string &name,
            const std::string &image,
            int cpu,
            int memory,
            const std::string &network
        );
        bool startContainerImpl(const std::string &name);
        bool stopContainerImpl(const std::string &name);
        bool destroyContainerImpl(const std::string &name);
    };
} // namespace nanoenv::containers
