cmake_minimum_required(VERSION 3.10)
project(nanoenv_api_containers CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -flto -fno-rtti -fno-exceptions")

find_package(Drogon CONFIG REQUIRED)

add_executable(nanoenv_api_containers
    src/container_controller.cpp)

target_include_directories(nanoenv_api_containers PRIVATE
    include)

target_link_libraries(nanoenv_api_containers PRIVATE
    Drogon::Drogon
    nanoenv_containers
    nanoenv_api_common
)