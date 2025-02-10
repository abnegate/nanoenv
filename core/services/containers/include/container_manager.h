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

        bool createEnvironment(
            const std::string &name,
            const std::string &image
        ) {
            return static_cast<T *>(this)->createEnvironmentImpl(name, image);
        }

        bool startEnvironment(
            const std::string &name
        ) {
            return static_cast<T *>(this)->startEnvironmentImpl(name);
        }

        bool stopEnvironment(
            const std::string &name
        ) {
            return static_cast<T *>(this)->stopEnvironmentImpl(name);
        }

        bool destroyEnvironment(
            const std::string &name
        ) {
            return static_cast<T *>(this)->destroyEnvironmentImpl(name);
        }

    private:
        ContainerManager() = default;
        ~ContainerManager() = default;

        friend T;
    };
} // namespace nanoenv::containers
