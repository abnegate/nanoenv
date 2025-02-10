cmake_minimum_required(VERSION 3.10)
project(nanoenv_cryptography)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSODIUM REQUIRED libsodium)

add_library(
    nanoenv_cryptography STATIC
    src/hashing.cpp
)

target_include_directories(
    nanoenv_cryptography PUBLIC
    include
    ${LIBSODIUM_INCLUDE_DIRS}
)

target_link_libraries(
    nanoenv_cryptography PRIVATE
    ${LIBSODIUM_LIBRARIES}
)

target_compile_options(
    nanoenv_cryptography PRIVATE
    ${LIBSODIUM_CFLAGS_OTHER}
)