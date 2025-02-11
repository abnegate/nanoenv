cmake_minimum_required(VERSION 3.24)
project(nanoenv CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -flto -fno-rtti -fno-exceptions")

set(CPM_DOWNLOAD_VERSION 0.40.5)
set(CPM_DOWNLOAD_PATH "${CMAKE_BINARY_DIR}/cmake/CPM.cmake")

if(NOT EXISTS "${CPM_DOWNLOAD_PATH}")
    message(STATUS "Downloading CPM.cmake ${CPM_DOWNLOAD_VERSION}")

    file(DOWNLOAD
        "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake"
        "${CPM_DOWNLOAD_PATH}"
        SHOW_PROGRESS
    )
endif()

include(${CPM_DOWNLOAD_PATH})

set(SODIUM_VERSION 1.0.19)
set(DROGON_VERSION 1.9.9)

find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(jsoncpp REQUIRED)

CPMAddPackage(
    NAME libsodium
    GITHUB_REPOSITORY jedisct1/libsodium
    GIT_TAG ${SODIUM_VERSION}
    OPTIONS
        "BUILD_SHARED_LIBS OFF"
        "SODIUM_DISABLE_TESTS ON"
        "CMAKE_POSITION_INDEPENDENT_CODE ON"
)
CPMAddPackage(
    NAME drogon
    GITHUB_REPOSITORY an-tao/drogon
    VERSION ${DROGON_VERSION}
    OPTIONS
        "BUILD_EXAMPLES OFF"
        "BUILD_TESTING OFF"
        "BUILD_DROGON_SHARED OFF"
)

add_subdirectory(core/services/auth)
add_subdirectory(core/services/containers)
add_subdirectory(core/services/cryptography)
add_subdirectory(core/services/database)
add_subdirectory(core/services/network)
add_subdirectory(core/services/storage)
add_subdirectory(core/services/system)

add_subdirectory(api/common)
add_subdirectory(api/containers)

add_custom_target(nanoenv DEPENDS
    nanoenv_auth
    nanoenv_containers
    nanoenv_cryptography
    nanoenv_database
    nanoenv_network
    nanoenv_storage
    nanoenv_system
    nanoenv_api_common
    nanoenv_api_containers
)
