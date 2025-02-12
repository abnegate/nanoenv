#pragma once

#include <functional>
#include <string>
#include <drogon/HttpController.h>
#include <drogon/HttpResponse.h>

using Callback = std::function<void(const drogon::HttpResponsePtr &)>;

namespace nanoenv::api {
    template <typename Derived, bool AutoCreation = true>
    class Controller : public drogon::HttpController<Derived, AutoCreation> {
    public:
        /**
         * @brief Sends a JSON response with the specified status code.
         *
         * @param result The JSON response to send.
         * @param code The HTTP status code to send.
         * @return The JSON response with the specified status code.
         */
        static drogon::HttpResponsePtr json(
            const Json::Value &result,
            const drogon::HttpStatusCode code = drogon::k200OK
        ) {
            const auto response = drogon::HttpResponse::newHttpJsonResponse(result);
            response->setStatusCode(code);
            response->setContentTypeCode(drogon::CT_APPLICATION_JSON);
            return response;
        }

        /**
         * @brief Sends an error response with the specified status code.
         *
         * @param message The error message to send.
         * @param code The HTTP status code to send.
         * @return The error response with the specified status code.
         */
        static drogon::HttpResponsePtr error(
            const std::string &message,
            const drogon::HttpStatusCode code
        ) {
            Json::Value error;
            error["error"] = message;
            auto response = drogon::HttpResponse::newHttpJsonResponse(error);
            response->setStatusCode(code);
            return response;
        }

    protected:
        static std::string getApiPrefix() {
            return "/api/v1";
        }
    };
} // namespace nanoenv::api
