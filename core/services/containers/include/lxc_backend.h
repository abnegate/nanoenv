#pragma once

#include "container_manager.h"
#include <string>

namespace nanoenv::containers {
    class LXCBackend : public ContainerManager<LXCBackend> {
    public:
        bool createEnvironmentImpl(const std::string& name, const std::string& image);
        bool startEnvironmentImpl(const std::string& name);
        bool stopEnvironmentImpl(const std::string& name);
        bool destroyEnvironmentImpl(const std::string& name);
    };
}