#pragma once

#include "rule.h"
#include <algorithm>
#include <format>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <drogon/drogon.h>

namespace nanoenv::api {
    template <typename T>
    struct Validator;

    template <>
    struct Validator<Int> {
        static std::optional<std::string> validate(
            const drogon::HttpRequestPtr &req,
            const Int &rule
        ) {
            const auto json = req->getJsonObject();
            if (!json || (rule.required && !json->isMember(rule.field))) {
                return std::format("Missing required parameter: '{}'", rule.field);
            }

            const auto &value = (*json)[rule.field];
            if (!value.isInt()) {
                return std::format("Parameter '{}' must be an integer.", rule.field);
            }

            const int val = value.asInt();
            if (val < rule.min || val > rule.max) {
                return std::format("Parameter '{}' must be between {} and {}.", rule.field, rule.min, rule.max);
            }

            return std::nullopt;
        }
    };

    template <>
    struct Validator<Double> {
        static std::optional<std::string> validate(
            const drogon::HttpRequestPtr &req,
            const Double &rule
        ) {
            const auto json = req->getJsonObject();
            if (!json || (rule.required && !json->isMember(rule.field))) {
                return std::format("Missing required parameter: '{}'", rule.field);
            }

            const auto &value = (*json)[rule.field];
            if (!value.isDouble()) {
                return std::format("Parameter '{}' must be a double.", rule.field);
            }

            const double val = value.asDouble();
            if (val < rule.min || val > rule.max) {
                return std::format("Parameter '{}' must be between {} and {}.", rule.field, rule.min, rule.max);
            }

            return std::nullopt;
        }
    };

    template <>
    struct Validator<Bool> {
        static std::optional<std::string> validate(
            const drogon::HttpRequestPtr &req,
            const Bool &rule
        ) {
            const auto json = req->getJsonObject();
            if (!json || (rule.required && !json->isMember(rule.field))) {
                return std::format("Missing required parameter: '{}'", rule.field);
            }

            if (!(*json)[rule.field].isBool()) {
                return std::format("Parameter '{}' must be a boolean.", rule.field);
            }

            return std::nullopt;
        }
    };

    template <>
    struct Validator<String> {
        static std::optional<std::string> validate(
            const drogon::HttpRequestPtr &req,
            const String &rule
        ) {
            const auto json = req->getJsonObject();
            if (!json || (rule.required && !json->isMember(rule.field))) {
                return std::format("Missing required parameter: '{}'", rule.field);
            }

            const auto &value = (*json)[rule.field];
            if (!value.isString()) {
                return std::format("Parameter '{}' must be a string.", rule.field);
            }

            const std::string val = value.asString();
            if (val.size() < rule.minLength || val.size() > rule.maxLength) {
                return std::format(
                    "Parameter '{}' must be between {} and {} characters long.",
                    rule.field,
                    rule.minLength,
                    rule.maxLength
                );
            }

            return std::nullopt;
        }
    };

    template <>
    struct Validator<StringWhitelist> {
        static std::optional<std::string> validate(
            const drogon::HttpRequestPtr &req,
            const StringWhitelist &rule
        ) {
            const auto json = req->getJsonObject();
            if (!json || (rule.required && !json->isMember(rule.field))) {
                return std::format("Missing required parameter: '{}'", rule.field);
            }

            const auto &value = (*json)[rule.field];
            if (!value.isString()) {
                return std::format("Parameter '{}' must be a string.", rule.field);
            }

            if (!std::ranges::contains(rule.whitelist, value.asString())) {
                return formatWhitelistError(rule.field, rule.whitelist);
            }

            return std::nullopt;
        }

    private:
        static constexpr std::string formatWhitelistError(
            std::string_view field,
            const std::span<const std::string_view> &whitelist
        ) {
            std::string result = std::format("Parameter '{}' must be one of: ", field);
            bool first = true;
            for (const auto &val : whitelist) {
                if (!first) {
                    result += ", ";
                }
                first = false;
                result += std::format("'{}'", val);
            }
            result += ".";
            return result;
        }
    };

    template <>
    struct Validator<IntWhitelist> {
        static std::optional<std::string> validate(
            const drogon::HttpRequestPtr &req,
            const IntWhitelist &rule
        ) {
            const auto json = req->getJsonObject();
            if (!json || (rule.required && !json->isMember(rule.field))) {
                return std::format("Missing required parameter: '{}'", rule.field);
            }

            const auto &value = (*json)[rule.field];
            if (!value.isInt()) {
                return std::format("Parameter '{}' must be an integer.", rule.field);
            }

            if (!std::ranges::contains(rule.whitelist, value.asInt())) {
                return formatWhitelistError(rule.field, rule.whitelist);
            }

            return std::nullopt;
        }

    private:
        static constexpr std::string formatWhitelistError(
            std::string_view field,
            const std::span<const int> &whitelist
        ) {
            std::string result = std::format("Parameter '{}' must be one of: ", field);
            bool first = true;
            for (const auto &val : whitelist) {
                if (!first) {
                    result += ", ";
                }
                first = false;
                result += std::format("{}", val);
            }
            result += ".";
            return result;
        }
    };
} // namespace nanoenv::api
