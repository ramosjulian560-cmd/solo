cmake_minimum_required(VERSION 3.16)

project(solo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Recursively find C++ files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "sources/*.cpp")

add_executable(solo ${SOURCES})

target_include_directories(solo PRIVATE headers)
