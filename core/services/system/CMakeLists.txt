cmake_minimum_required(VERSION 3.10)
project(nanoenv_system)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(
    nanoenv_system STATIC
    src/system.cpp
)

target_include_directories(
    nanoenv_system PUBLIC
    include
)