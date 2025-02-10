#include "lxc_backend.h"
#include <iostream>
#include <memory>

namespace nanoenv::containers {
bool LXCBackend::createEnvironmentImpl(const std::string& name, const std::string& image) {
    auto container = std::unique_ptr<lxc_container, decltype(&lxc_container_free)>(
        lxc_container_new(name.c_str(), nullptr), lxc_container_free
    );
    return container && container->createl(container.get(), "download", nullptr, nullptr,
                                           LXC_CREATE_QUIET, "-d", image.c_str(), "-r", "22.04", "-a", "amd64", nullptr);
}

bool LXCBackend::startEnvironmentImpl(const std::string& name) {
    auto container = std::unique_ptr<lxc_container, decltype(&lxc_container_free)>(
        lxc_container_new(name.c_str(), nullptr), lxc_container_free
    );
    return container && container->start(container.get(), 0, nullptr);
}

bool LXCBackend::stopEnvironmentImpl(const std::string& name) {
    auto container = std::unique_ptr<lxc_container, decltype(&lxc_container_free)>(
        lxc_container_new(name.c_str(), nullptr), lxc_container_free
    );
    return container && container->shutdown(container.get(), 30);
}

bool LXCBackend::destroyEnvironmentImpl(const std::string& name) {
    auto container = std::unique_ptr<lxc_container, decltype(&lxc_container_free)>(
        lxc_container_new(name.c_str(), nullptr), lxc_container_free
    );
    return container && container->destroy(container.get());
}
}