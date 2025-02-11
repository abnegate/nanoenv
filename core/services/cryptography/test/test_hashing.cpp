#include "hashing.h"
#include <gtest/gtest.h>
#include <sodium.h>

using namespace nanoenv::cryptography;

// Ensure libsodium is initialized before running tests
class HashingTest : public testing::Test {
protected:
    static void SetUpTestSuite() {
        if (sodium_init() < 0) {
            throw std::runtime_error("Failed to initialize libsodium");
        }
    }
};

TEST_F(
    HashingTest,
    GeneratesValidHash
) {
    const std::string input = "password123";
    const std::string hash = Hashing::hashArgon2(input);

    EXPECT_FALSE(hash.empty()) << "Hash should not be empty";
    EXPECT_TRUE(hash.size() >= crypto_pwhash_STRBYTES - 1) << "Hash should be at least the expected length";
}

TEST_F(
    HashingTest,
    HashesAreDifferentForDifferentInputs
) {
    const std::string hash1 = Hashing::hashArgon2("password123");
    const std::string hash2 = Hashing::hashArgon2("password124");

    EXPECT_NE(hash1, hash2) << "Hashes of different inputs should not be the same";
}

TEST_F(
    HashingTest,
    HashCanBeVerified
) {
    const std::string password = "password123";
    const std::string hash = Hashing::hashArgon2(password);

    // Verify the hash
    const bool result = Hashing::verifyArgon2(hash, password);
    EXPECT_TRUE(result) << "Hash should verify successfully";
}

TEST_F(
    HashingTest,
    IncorrectPasswordFailsVerification
) {
    const std::string correctPassword = "password123";
    const std::string incorrectPassword = "password124";
    const std::string hash = Hashing::hashArgon2(correctPassword);

    const bool result = Hashing::verifyArgon2(hash, incorrectPassword);
    EXPECT_FALSE(result) << "Incorrect password should not verify";
}
