cmake_minimum_required(VERSION 3.24)
project(nanoenv CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -flto -fno-rtti -fno-exceptions")

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
