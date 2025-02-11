cmake_minimum_required(VERSION 3.24)
project(nanoenv_api_containers_tests CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

add_executable(
    nanoenv_api_containers_tests
    test_api_containers.cpp
)

target_link_libraries(
    nanoenv_api_containers_tests PRIVATE
    nanoenv_api_containers_lib
    GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(nanoenv_api_containers_tests)