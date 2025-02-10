#include "hashing.h"
#include <array>
#include <sodium.h>

namespace nanoenv::cryptography {
    std::string Hashing::hashArgon2(
        const std::string &input
    ) {
        std::array<char, crypto_pwhash_STRBYTES> hashedKey{};

        if (crypto_pwhash_str(
                hashedKey.data(),                   // Output hash
                input.c_str(),                      // Input API key
                input.length(),                     // Length of the key
                crypto_pwhash_OPSLIMIT_INTERACTIVE, // Computational cost
                crypto_pwhash_MEMLIMIT_INTERACTIVE  // Memory cost
            ) != 0) {
            throw std::runtime_error("Failed to hash API key");
        }

        return {hashedKey.data()};
    }
} // namespace nanoenv::cryptography
