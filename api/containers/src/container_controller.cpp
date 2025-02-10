#include "container_controller.h"
#include "container_manager.h"
#include "platform.h"
#include "validator.h"
#include <string>
#include <thread>
#include <drogon/drogon.h>

using namespace nanoenv::containers;

namespace nanoenv::api::containers {
    void ContainerController::create(
        const drogon::HttpRequestPtr &req,
        Callback &&callback
    ) {
        if (const auto message = validate(req, getCreateRules())) {
            callback(error(*message, drogon::k400BadRequest));
            return;
        }

        const auto request = req->getJsonObject();
        const std::string name = (*request)["name"].asString();
        const std::string image = (*request)["image"].asString();
        const int cpu = (*request)["cpu"].asInt();
        const int memory = (*request)["memory"].asInt();
        const std::string network = (*request)["network"].asString();

        if (!ContainerManager<Backend>::getInstance().createEnvironment(name, image)) {
            callback(error("Failed to create container.", drogon::k500InternalServerError));
            return;
        }

        callback(json(*request));
    }

    void ContainerController::start(
        const drogon::HttpRequestPtr &req,
        Callback &&callback,
        const std::string &containerId
    ) {
        if (!ContainerManager<Backend>::getInstance().startEnvironment(containerId)) {
            callback(error("Failed to start container.", drogon::k500InternalServerError));
            return;
        }

        callback(json(Json::Value{}));
    }

    void ContainerController::stop(
        const drogon::HttpRequestPtr &req,
        Callback &&callback,
        const std::string &containerId
    ) {
        if (!ContainerManager<Backend>::getInstance().stopEnvironment(containerId)) {
            callback(error("Failed to stop container.", drogon::k500InternalServerError));
            return;
        }

        callback(json(Json::Value{}));
    }

    void ContainerController::destroy(
        const drogon::HttpRequestPtr &req,
        Callback &&callback,
        const std::string &containerId
    ) {
        if (!ContainerManager<Backend>::getInstance().destroyEnvironment(containerId)) {
            callback(error("Failed to destroy container.", drogon::k500InternalServerError));
            return;
        }

        callback(json(Json::Value{}));
    }
}

int main() {
    drogon::app()
        .addListener("0.0.0.0", 8080)
        .setLogPath("./")
        .setLogLevel(trantor::Logger::kInfo)
        .setThreadNum(std::thread::hardware_concurrency())
        .run();
}
