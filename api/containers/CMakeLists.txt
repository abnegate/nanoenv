cmake_minimum_required(VERSION 3.10)
project(nanoenv_api_containers CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -flto -fno-rtti -fno-exceptions")

add_library(
    nanoenv_api_containers_lib STATIC
    src/container_create_req.cpp
    src/container_controller.cpp
)

target_include_directories(
    nanoenv_api_containers_lib PUBLIC
    include
)

target_link_libraries(
    nanoenv_api_containers_lib PUBLIC
    nanoenv_api_common
    nanoenv_containers
    nanoenv_system
    drogon
    simdjson
)

add_executable(
    nanoenv_api_containers
    src/main.cpp
)

target_link_libraries(
    nanoenv_api_containers PRIVATE
    nanoenv_api_containers_lib
)