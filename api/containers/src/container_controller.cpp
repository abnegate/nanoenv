#include "container_controller.h"
#include "container_create_req.h"
#include "container_manager.h"
#include "platform.h"
#include <string>
#include <drogon/drogon.h>

using namespace nanoenv::containers;

namespace nanoenv::api::containers {
    void ContainerController::create(
        const HttpRequestPtr &req,
        Callback &&callback
    ) const {
        const auto request = ContainerCreateReq::fromRequest(*req);
        if (!request) {
            callback(error(request.error(), k400BadRequest));
            return;
        }

        const auto [name, image, cpu, memory, network] = *request;

        if (!containerManager->createContainer(name, image, cpu, memory, network)) {
            callback(error("Failed to create container.", k500InternalServerError));
            return;
        }

        callback(json(Json::Value{}));
    }

    void ContainerController::start(
        const HttpRequestPtr &req,
        Callback &&callback,
        const std::string &containerId
    ) {
        if (!ContainerManager<Backend>::getInstance().startContainer(containerId)) {
            callback(error("Failed to start container.", k500InternalServerError));
            return;
        }

        callback(json(Json::Value{}));
    }

    void ContainerController::stop(
        const HttpRequestPtr &req,
        Callback &&callback,
        const std::string &containerId
    ) {
        if (!ContainerManager<Backend>::getInstance().stopContainer(containerId)) {
            callback(error("Failed to stop container.", k500InternalServerError));
            return;
        }

        callback(json(Json::Value{}));
    }

    void ContainerController::destroy(
        const HttpRequestPtr &req,
        Callback &&callback,
        const std::string &containerId
    ) {
        if (!ContainerManager<Backend>::getInstance().destroyContainer(containerId)) {
            callback(error("Failed to destroy container.", k500InternalServerError));
            return;
        }

        callback(json(Json::Value{}));
    }

    void ContainerController::health(
        const HttpRequestPtr &req,
        Callback &&callback
    ) {
        Json::Value response;
        response["status"] = "ok";

        callback(json(response));
    }

} // namespace nanoenv::api::containers
