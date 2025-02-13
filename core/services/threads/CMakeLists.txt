cmake_minimum_required(VERSION 3.10)
project(nanoenv_threads CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(nanoenv_threads INTERFACE)

target_include_directories(
    nanoenv_threads INTERFACE
    include
)