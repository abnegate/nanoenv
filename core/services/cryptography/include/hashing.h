#pragma once

#include <string>

namespace nanoenv::cryptography {
    class Hashing {
    public:
        static std::string hashArgon2(const std::string &input);
    };
} // namespace nanoenv::cryptography
