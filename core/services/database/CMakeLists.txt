cmake_minimum_required(VERSION 3.10)
project(nanoenv_database)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(
    nanoenv_database STATIC
    src/database.cpp
)

target_include_directories(
    nanoenv_database PUBLIC
    include
)

target_link_libraries(
    nanoenv_database PUBLIC
    nanoenv_system
    drogon
)