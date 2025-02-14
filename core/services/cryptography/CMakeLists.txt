cmake_minimum_required(VERSION 3.10)
project(nanoenv_cryptography)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(
    nanoenv_cryptography STATIC
    src/hashing.cpp
)

target_include_directories(
    nanoenv_cryptography PUBLIC
    include
    ${SODIUM_INCLUDE_DIRS}
)

target_link_libraries(
    nanoenv_cryptography PRIVATE
    ${SODIUM_LIBRARIES}
)