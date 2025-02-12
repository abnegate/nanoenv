#pragma once

#include <cctype>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <string>

#if defined(__i386__) || defined(__x86_64__)
#include <immintrin.h>
#endif

#if defined(__AVX2__)
#define USE_AVX2
#elif defined(__ARM_NEON)
#define USE_NEON
#else
#define USE_SCALAR
#endif

namespace nanoenv::system {
    class System {
    public:

        /**
         * @brief Gets the current system time in ISO 8601 format.
         *
         * @return The current system time in ISO 8601 format.
         */
        static std::string getCurrentTime();

        /**
         * @brief Generates a random UUID.
         *
         * @return A random UUID.
         */
        static std::string generateUUID();

        /**
         * @brief Gets the value of the specified environment variable.
         *
         * @tparam T The type of the environment variable.
         * @param name The name of the environment variable.
         * @param defaultValue The default value to return if the environment variable is not set.
         * @return The value of the environment variable, or the default value if not set.
         */
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
} // namespace nanoenv::system
