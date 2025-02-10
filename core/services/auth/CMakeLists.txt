cmake_minimum_required(VERSION 3.10)

project(nanoenv_auth LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(nanoenv_auth STATIC)

target_sources(nanoenv_auth PRIVATE
    src/auth.cpp)

target_include_directories(nanoenv_auth PUBLIC
    include)