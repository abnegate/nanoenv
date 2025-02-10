#pragma once

#include "controller.h"
#include <array>
#include <string>
#include <string_view>
#include <drogon/HttpController.h>

namespace nanoenv::api::containers {
    inline constexpr std::array<std::string_view, 3> networkModes = {"bridge", "host", "none"};
    inline constexpr std::array<std::string_view, 4> storageTypes = {"local", "nfs", "s3", "tmpfs"};
    inline constexpr std::array<std::string_view, 2> logLevels = {"debug", "error"};

    class ContainerController final : public Controller<ContainerController> {
    public:
        METHOD_LIST_BEGIN
        ADD_METHOD_TO(ContainerController::create, "/containers", drogon::Post);
        ADD_METHOD_TO(ContainerController::start, "/containers/{1}/start", drogon::Post);
        ADD_METHOD_TO(ContainerController::stop, "/containers/{1}/stop", drogon::Post);
        ADD_METHOD_TO(ContainerController::destroy, "/containers/{1}", drogon::Delete);
        METHOD_LIST_END

        static void create(const drogon::HttpRequestPtr &req, Callback &&callback);

        static void start(const drogon::HttpRequestPtr &req, Callback &&callback, const std::string &containerId);

        static void stop(const drogon::HttpRequestPtr &req, Callback &&callback, const std::string &containerId);

        static void destroy(const drogon::HttpRequestPtr &req, Callback &&callback, const std::string &containerId);

    protected:
        static constexpr std::array<AnyRule, 5> getCreateRules() {
            return std::array<AnyRule, 5>{
                {String{"name", true},
                 String{"image", true},
                 Int{"cpu", true, 1, 16},
                 Int{"memory", true, 512, 64000},
                 StringWhitelist{"network", true, networkModes}}
            };
        }

    private:
        static std::string generateContainerId();
    };
} // namespace nanoenv::api::containers
