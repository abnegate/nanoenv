#pragma once

#include <cctype>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <string>

#if defined(__AVX2__)
#define USE_AVX2
#elif defined(__ARM_NEON)
#define USE_NEON
#else
#define USE_SCALAR
#endif

namespace nanoenv::platform {
    class System {
    public:
        static std::string getCurrentTime();

        template <typename T>
        static T getVariable(
            const std::string &name,
            const T &defaultValue
        ) {
            const char *value = std::getenv(name.c_str());
            if (!value) {
                return defaultValue;
            }

            std::stringstream stream(value);

            T result;

            if constexpr (std::is_same_v<T, bool>) {
                auto boolStr = std::string(value);

                std::transform(boolStr.begin(), boolStr.end(), boolStr.begin(), [](const unsigned char character) {
                    return std::tolower(character);
                });

                if (boolStr == "true" || boolStr == "1") {
                    return true;
                }
                if (boolStr == "false" || boolStr == "0") {
                    return false;
                }

                throw std::invalid_argument("Invalid value for boolean environment variable: " + std::string(value));
            }

            if (!(stream >> result)) {
                throw std::invalid_argument("Failed to convert environment variable " + name + " to the required type");
            }

            return result;
        }
    };
} // namespace nanoenv::platform