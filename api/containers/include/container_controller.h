#pragma once

#include "controller.h"
#include <container_manager.h>
#include <string>
#include <drogon/HttpController.h>
#include <platform.h>

using namespace drogon;
using namespace nanoenv::containers;

namespace nanoenv::api::containers {
    class ContainerController final : public Controller<ContainerController, false> {
    public:
        METHOD_LIST_BEGIN
        ADD_METHOD_TO(ContainerController::create, "/containers", Post);
        ADD_METHOD_TO(ContainerController::start, "/containers/{1}/start", Post);
        ADD_METHOD_TO(ContainerController::stop, "/containers/{1}/stop", Post);
        ADD_METHOD_TO(ContainerController::destroy, "/containers/{1}", Delete);
        ADD_METHOD_TO(ContainerController::health, "/containers/health", Get);
        METHOD_LIST_END

        /**
         * @brief Creates a new container from the given request.
         *
         * @param req The HTTP request containing the container configuration.
         * @param callback The callback to invoke upon completion.
         */
        void create(const HttpRequestPtr &req, Callback &&callback) const;

        /**
         * @brief Starts the container with the specified ID.
         *
         * @param req The HTTP request containing the container ID.
         * @param callback The callback to invoke upon completion.
         * @param containerId The ID of the container to start.
         */
        void start(const HttpRequestPtr &req, Callback &&callback, const std::string &containerId);

        /**
         * @brief Stops the container with the specified ID.
         *
         * @param req The HTTP request containing the container ID.
         * @param callback The callback to invoke upon completion.
         * @param containerId The ID of the container to stop.
         */
        void stop(const HttpRequestPtr &req, Callback &&callback, const std::string &containerId);

        /**
         * @brief Destroys the container with the specified ID.
         *
         * @param req The HTTP request containing the container ID.
         * @param callback The callback to invoke upon completion.
         * @param containerId The ID of the container to destroy.
         */
        void destroy(const HttpRequestPtr &req, Callback &&callback, const std::string &containerId);

        /**
         * @brief Checks the health of the container service.
         *
         * @param req The HTTP request to check the health of the service.
         * @param callback The callback to invoke upon completion.
         */
        static void health(const HttpRequestPtr &req, Callback &&callback);

    private:
        ContainerManager<Backend> *containerManager = &ContainerManager<Backend>::getInstance();
    };
} // namespace nanoenv::api::containers
