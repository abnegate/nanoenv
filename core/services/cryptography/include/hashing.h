#pragma once

#include <string>

namespace nanoenv::cryptography {
    class Hashing {
    public:
        /**
         * @brief Hashes the given input using the Argon2 algorithm.
         *
         * @param input The input to hash.
         * @return The hashed input.
         */
        static std::string hashArgon2(const std::string &input);

        /**
         * @brief Verifies the given input against the given Argon2 hash.
         *
         * @param hash The Argon2 hash to verify against.
         * @param input The input to verify.
         * @return True if the input matches the hash, false otherwise.
         */
        static bool verifyArgon2(const std::string &hash, const std::string &input);
    };
} // namespace nanoenv::cryptography
