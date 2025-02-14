cmake_minimum_required(VERSION 3.24)
project(nanoenv CXX)

option(BUILD_TESTING "Build tests" ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -flto=auto -frtti")

set(CPM_DOWNLOAD_VERSION 0.40.5)
set(CPM_DOWNLOAD_PATH "${CMAKE_BINARY_DIR}/cmake/CPM.cmake")

if (NOT EXISTS "${CPM_DOWNLOAD_PATH}")
    message(STATUS "Downloading CPM.cmake ${CPM_DOWNLOAD_VERSION}")

    file(
        DOWNLOAD "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake"
        "${CPM_DOWNLOAD_PATH}"
        SHOW_PROGRESS
    )
endif ()

include(${CPM_DOWNLOAD_PATH})

set(LIBSODIUM_VERSION 1.0.19)
set(DROGON_VERSION 1.9.9)
set(SIMDJSON_VERSION 3.12.1)
set(LIBARCHIVE_VERSION 3.7.7)
set(LIBPSL_VERSION 0.21.5)
set(CURL_VERSION 8_12_0)
set(LIBURING_VERSION 2.9)
set(GTEST_VERSION 1.16.0)

find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(jsoncpp REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED libsodium)

CPMAddPackage(
    NAME libsodium
    GITHUB_REPOSITORY jedisct1/libsodium
    GIT_TAG ${LIBSODIUM_VERSION}
)
CPMAddPackage(
    NAME drogon
    GITHUB_REPOSITORY an-tao/drogon
    VERSION ${DROGON_VERSION}
)
CPMAddPackage(
    NAME simdjson
    GITHUB_REPOSITORY simdjson/simdjson
    VERSION ${SIMDJSON_VERSION}
)
CPMAddPackage(
    NAME gtest
    GITHUB_REPOSITORY google/googletest
    GIT_TAG v${GTEST_VERSION}
)
CPMAddPackage(
    NAME libarchive
    GITHUB_REPOSITORY libarchive/libarchive
    GIT_TAG v${LIBARCHIVE_VERSION}
)
CPMAddPackage(
    NAME libpsl
    GITHUB_REPOSITORY rockdaboot/libpsl
    GIT_TAG ${LIBPSL_VERSION}
)
CPMAddPackage(
    NAME curl
    GITHUB_REPOSITORY curl/curl
    GIT_TAG curl-${CURL_VERSION}
)
CPMAddPackage(
    NAME liburing
    GITHUB_REPOSITORY axboe/liburing
    GIT_TAG liburing-${LIBURING_VERSION}
)

add_subdirectory(core/services/auth)
add_subdirectory(core/services/containers)
add_subdirectory(core/services/cryptography)
add_subdirectory(core/services/database)
add_subdirectory(core/services/network)
add_subdirectory(core/services/storage)
add_subdirectory(core/services/system)
add_subdirectory(core/services/threads)

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

if (BUILD_TESTING)
    enable_testing()
    add_subdirectory(core/services/auth/test)
    add_subdirectory(core/services/containers/test)
    add_subdirectory(core/services/cryptography/test)
    add_subdirectory(core/services/database/test)
    add_subdirectory(core/services/network/test)
    add_subdirectory(core/services/storage/test)
    add_subdirectory(core/services/system/test)

    add_subdirectory(api/containers/test)
endif ()
