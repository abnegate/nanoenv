cmake_minimum_required(VERSION 3.10)
project(nanoenv_api_common CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(nanoenv_api_common INTERFACE)

target_include_directories(
    nanoenv_api_common INTERFACE
    include
)

target_link_libraries(
    nanoenv_api_common INTERFACE
    drogon
)