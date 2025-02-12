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

        void create(const HttpRequestPtr &req, Callback &&callback) const;

        void start(const HttpRequestPtr &req, Callback &&callback, const std::string &containerId);

        void stop(const HttpRequestPtr &req, Callback &&callback, const std::string &containerId);

        void destroy(const HttpRequestPtr &req, Callback &&callback, const std::string &containerId);

        static void health(const HttpRequestPtr &req, Callback &&callback);

    private:
        ContainerManager<Backend> *containerManager = &ContainerManager<Backend>::getInstance();
    };
} // namespace nanoenv::api::containers
