#include "container_controller.h"
#include <thread>
#include <drogon/drogon.h>

int main() {
    const auto controller = std::make_shared<nanoenv::api::containers::ContainerController>();

    drogon::app()
        .addListener("0.0.0.0", 18080)
        .setLogPath("./")
        .setLogLevel(trantor::Logger::kInfo)
        .setThreadNum(std::thread::hardware_concurrency())
        .registerController(controller)
        .run();
}
