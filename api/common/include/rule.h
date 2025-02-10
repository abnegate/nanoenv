#pragma once

#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <variant>

namespace nanoenv::api {

    struct Rule {
        std::string &&field;
        bool required;
    };

    template <typename>
    struct ValidationRule : Rule {};

    template <>
    struct ValidationRule<int> : Rule {
        int min;
        int max;

        explicit constexpr ValidationRule(
            std::string &&field,
            const bool required,
            const int min = std::numeric_limits<int>::min(),
            const int max = std::numeric_limits<int>::max()
        ) :
            Rule(std::move(field), required),
            min(min),
            max(max) {}
    };

    template <>
    struct ValidationRule<double> : Rule {
        double min;
        double max;

        explicit constexpr ValidationRule(
            std::string &&field,
            const bool required,
            const double min = std::numeric_limits<double>::min(),
            const double max = std::numeric_limits<double>::max()
        ) :
            Rule(std::move(field), required),
            min(min),
            max(max) {}
    };

    template <>
    struct ValidationRule<bool> : Rule {
        explicit constexpr ValidationRule(
            std::string &&field,
            const bool required
        ) :
            Rule(std::move(field), required) {}
    };

    template <>
    struct ValidationRule<std::string> : Rule {
        int minLength;
        int maxLength;

        explicit constexpr ValidationRule(
            std::string &&field,
            const bool required,
            const int minLength = 0,
            const int maxLength = std::numeric_limits<int>::max()
        ) :
            Rule(std::move(field), required),
            minLength(minLength),
            maxLength(maxLength) {}
    };

    template <>
    struct ValidationRule<std::span<const int>> : Rule {
        std::span<const int> whitelist;

        explicit constexpr ValidationRule(
            std::string &&field,
            const bool required,
            const std::span<const int> values
        ) :
            Rule(std::move(field), required),
            whitelist(values) {}
    };

    template <>
    struct ValidationRule<std::span<const std::string_view>> : Rule {
        std::span<const std::string_view> whitelist;

        explicit constexpr ValidationRule(
            std::string &&field,
            const bool required,
            const std::span<const std::string_view> values
        ) :
            Rule(std::move(field), required),
            whitelist(values) {}
    };

    using Int = ValidationRule<int>;
    using Double = ValidationRule<double>;
    using Bool = ValidationRule<bool>;
    using String = ValidationRule<std::string>;
    using IntWhitelist = ValidationRule<std::span<const int>>;
    using StringWhitelist = ValidationRule<std::span<const std::string_view>>;

    using AnyRule = std::variant<Int, Double, Bool, String, IntWhitelist, StringWhitelist>;
} // namespace nanoenv::api
