#pragma once

#include <string>

namespace nanoenv::cryptography {
    class Hashing {
    public:
        static std::string hashArgon2(const std::string &input);
        static bool verifyArgon2(const std::string &hash, const std::string &input);
    };
} // namespace nanoenv::cryptography
