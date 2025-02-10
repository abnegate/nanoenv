#pragma once

#include "rule.h"
#include "validator.h"

namespace nanoenv::api {
    template <size_t NRules>
    class RequestValidator {
    public:
        static constexpr std::optional<std::string> validate(
            const drogon::HttpRequestPtr &req,
            const std::array<AnyRule, NRules> &rules
        ) {
            for (const auto &rule : rules) {
                std::optional<std::string> error = std::visit(
                    [&]<typename TRule>(const TRule &r) -> std::optional<std::string> {
                        return Validator<std::decay_t<TRule>>::validate(req, r);
                    },
                    rule
                );

                if (error) {
                    return error;
                }
            }

            return std::nullopt;
        }
    };
} // namespace nanoenv::api
