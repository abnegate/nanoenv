cmake_minimum_required(VERSION 3.10)

project(nanoenv_storage LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(nanoenv_storage STATIC)

target_sources(nanoenv_storage PRIVATE
    src/storage_manager.cpp)

target_include_directories(nanoenv_storage PUBLIC
    include)