cmake_minimum_required(VERSION 3.24)
project(nanoenv_database_tests CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

add_executable(
    nanoenv_database_tests
    test_database.cpp
)

target_link_libraries(
    nanoenv_database_tests PRIVATE
    nanoenv_system
    GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(nanoenv_database_tests)