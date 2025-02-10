cmake_minimum_required(VERSION 3.10)

project(nanoenv_network LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(nanoenv_network STATIC)

target_sources(nanoenv_network PRIVATE
    src/network_manager.cpp)

target_include_directories(nanoenv_network PUBLIC
    include)