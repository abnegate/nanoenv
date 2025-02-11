cmake_minimum_required(VERSION 3.24)
project(nanoenv_containers_tests CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

add_executable(
    nanoenv_containers_tests
    test_containers.cpp
)

target_link_libraries(
    nanoenv_containers_tests PRIVATE
    nanoenv_system
    GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(nanoenv_containers_tests)