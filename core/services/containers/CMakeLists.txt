cmake_minimum_required(VERSION 3.10)

project(nanoenv_containers LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(nanoenv_containers STATIC)

target_sources(
    nanoenv_containers PRIVATE
    src/oci_image.cpp
)

if (APPLE)
    target_sources(
        nanoenv_containers PRIVATE
        src/lima_backend.cpp
    )
#else ()
    target_sources(
        nanoenv_containers PRIVATE
        src/lxc_backend.cpp
    )
endif ()

target_include_directories(
    nanoenv_containers PUBLIC
    include
    ${libarchive_SOURCE_DIR}/libarchive
)

target_link_libraries(
    nanoenv_containers PUBLIC
    nanoenv_system
    nanoenv_threads
    libarchive
    simdjson
)