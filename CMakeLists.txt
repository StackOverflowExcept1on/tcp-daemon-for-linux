cmake_minimum_required(VERSION 3.11)
project(tcp-daemon-for-linux)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_subdirectory(client)
add_subdirectory(server)
