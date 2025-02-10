#pragma once

#include "request_validator.h"
#include "rule.h"
#include "validator.h"
#include <functional>
#include <string>
#include <drogon/HttpController.h>
#include <drogon/HttpResponse.h>
#include <json/json.h>

using Callback = std::function<void(const drogon::HttpResponsePtr &)>;

namespace nanoenv::api {
    template <typename Derived, bool AutoCreation = true>
    class Controller : public drogon::HttpController<Derived, AutoCreation> {
    public:
        static drogon::HttpResponsePtr json(
            const Json::Value &result,
            const drogon::HttpStatusCode code = drogon::k200OK
        ) {
            const auto response = drogon::HttpResponse::newHttpJsonResponse(result);
            response->setStatusCode(code);
            response->setContentTypeCode(drogon::CT_APPLICATION_JSON);
            return response;
        }

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

        template <size_t N>
        static std::optional<std::string> validate(
            const drogon::HttpRequestPtr &req,
            const std::array<AnyRule, N> &rules
        ) {
            if (const auto message = RequestValidator<N>::validate(req, rules)) {
                return message;
            }
            return std::nullopt;
        }

    protected:
        static std::string getApiPrefix() {
            return "/api/v1";
        }
    };
} // namespace nanoenv::api
