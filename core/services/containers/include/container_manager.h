#pragma once

#include <string>

namespace nanoenv::containers {
    template <class T>
    class ContainerManager {
    public:
        ContainerManager(const ContainerManager &) = delete;

        ContainerManager &operator=(const ContainerManager &) = delete;

        static ContainerManager &getInstance() {
            static ContainerManager instance;
            return instance;
        }

        bool createContainer(
            const std::string &name,
            const std::string &image
        ) {
            return static_cast<T *>(this)->createContainerImpl(name, image);
        }

        bool startContainer(
            const std::string &name
        ) {
            return static_cast<T *>(this)->startContainerImpl(name);
        }

        bool stopContainer(
            const std::string &name
        ) {
            return static_cast<T *>(this)->stopContainerImpl(name);
        }

        bool destroyContainer(
            const std::string &name
        ) {
            return static_cast<T *>(this)->destroyContainerImpl(name);
        }

    private:
        ContainerManager() = default;
        ~ContainerManager() = default;

        friend T;
    };
} // namespace nanoenv::containers
