#include "system.h"
#include <regex>
#include <gtest/gtest.h>

using namespace nanoenv::system;

TEST(
    SystemTest,
    GetCurrentTime
) {
    const std::string time = System::getCurrentTime();

    // Regex to validate ISO 8601 format with milliseconds: YYYY-MM-DDTHH:MM:SS.sssZ
    const std::regex regex(R"(\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}\.\d{3}Z)");
    EXPECT_TRUE(std::regex_match(time, regex)) << "Invalid time format: " << time;
}

TEST(
    SystemTest,
    GetVariableInt
) {
    setenv("TEST_INT", "42", 1);
    EXPECT_EQ(System::getVariable<int>("TEST_INT", 0), 42);
}

TEST(
    SystemTest,
    GetVariableDouble
) {
    setenv("TEST_DOUBLE", "3.14", 1);
    EXPECT_DOUBLE_EQ(System::getVariable<double>("TEST_DOUBLE", 0.0), 3.14);
}

TEST(
    SystemTest,
    GetVariableBoolTrue
) {
    setenv("TEST_BOOL_TRUE", "true", 1);
    EXPECT_TRUE(System::getVariable<bool>("TEST_BOOL_TRUE", false));

    setenv("TEST_BOOL_ONE", "1", 1);
    EXPECT_TRUE(System::getVariable<bool>("TEST_BOOL_ONE", false));
}

TEST(
    SystemTest,
    GetVariableBoolFalse
) {
    setenv("TEST_BOOL_FALSE", "false", 1);
    EXPECT_FALSE(System::getVariable<bool>("TEST_BOOL_FALSE", true));

    setenv("TEST_BOOL_ZERO", "0", 1);
    EXPECT_FALSE(System::getVariable<bool>("TEST_BOOL_ZERO", true));
}

TEST(
    SystemTest,
    GetVariableBoolInvalid
) {
    setenv("TEST_BOOL_INVALID", "notabool", 1);
    EXPECT_THROW(System::getVariable<bool>("TEST_BOOL_INVALID", false), std::invalid_argument);
}

TEST(
    SystemTest,
    GetVariableDefault
) {
    unsetenv("NON_EXISTENT_ENV");
    EXPECT_EQ(System::getVariable<int>("NON_EXISTENT_ENV", 99), 99);
}

TEST(
    SystemTest,
    GetVariableInvalidType
) {
    setenv("TEST_INVALID_INT", "hello", 1);
    EXPECT_THROW(System::getVariable<int>("TEST_INVALID_INT", 0), std::invalid_argument);
}
