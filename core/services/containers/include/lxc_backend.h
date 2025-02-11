#pragma once

#include "container_manager.h"
#include <string>

namespace nanoenv::containers {
    class LXCBackend : public ContainerManager<LXCBackend> {
    public:
        bool createContainerImpl(const std::string& name, const std::string& image);
        bool startContainerImpl(const std::string& name);
        bool stopContainerImpl(const std::string& name);
        bool destroyContainerImpl(const std::string& name);
    };
}