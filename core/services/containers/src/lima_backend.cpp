#include "lima_backend.h"
#include <cstdlib>
#include <iostream>

namespace nanoenv::containers {
    bool LimaBackend::createContainerImpl(
        const std::string &name,
        const std::string &image
    ) {
        const std::string command =
            "limactl shell default lxc-create -n " + name + " -t download -- -d " + image + " -r 22.04 -a amd64";
        return std::system(command.c_str()) == 0;
    }

    bool LimaBackend::startContainerImpl(
        const std::string &name
    ) {
        const std::string command = "limactl shell default lxc-start -n " + name;
        return std::system(command.c_str()) == 0;
    }

    bool LimaBackend::stopContainerImpl(
        const std::string &name
    ) {
        const std::string command = "limactl shell default lxc-stop -n " + name;
        return std::system(command.c_str()) == 0;
    }

    bool LimaBackend::destroyContainerImpl(
        const std::string &name
    ) {
        const std::string command = "limactl shell default lxc-destroy -n " + name;
        return std::system(command.c_str()) == 0;
    }
} // namespace nanoenv::containers
