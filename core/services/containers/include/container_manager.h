#pragma once

#include <string>

namespace nanoenv::containers {
    template <class T>
    class ContainerManager {
    public:
        ContainerManager(const ContainerManager &) = delete;
        ContainerManager &operator=(const ContainerManager &) = delete;

        /**
         * @brief Gets the singleton instance of the container manager.
         *
         * @return The singleton instance of the container manager.
         */
        static ContainerManager &getInstance() {
            static ContainerManager instance;
            return instance;
        }

        /**
         * @brief Creates a new container with the specified configuration.
         *
         * @param name The name of the container.
         * @param image The OCI image to use.
         * @param cpu The number of CPU cores to allocate.
         * @param memory The amount of memory to allocate (in MB).
         * @param network The network mode to use.
         * @return True if the container was successfully created, false otherwise.
         */
        bool createContainer(
            const std::string &name,
            const std::string &image,
            const int cpu,
            const int memory,
            const std::string &network
        ) {
            return static_cast<T *>(this)->createContainerImpl(name, image, cpu, memory, network);
        }

        /**
         * @brief Starts the container with the specified name.
         *
         * @param name The name of the container to start.
         * @return True if the container was successfully started, false otherwise.
         */
        bool startContainer(
            const std::string &name
        ) {
            return static_cast<T *>(this)->startContainerImpl(name);
        }

        /**
         * @brief Stops the container with the specified name.
         *
         * @param name The name of the container to stop.
         * @return True if the container was successfully stopped, false otherwise.
         */
        bool stopContainer(
            const std::string &name
        ) {
            return static_cast<T *>(this)->stopContainerImpl(name);
        }

        /**
         * @brief Destroys the container with the specified name.
         *
         * @param name The name of the container to destroy.
         * @return True if the container was successfully destroyed, false otherwise.
         */
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
