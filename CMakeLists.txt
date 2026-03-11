cmake_minimum_required(VERSION 3.5)
project(Etched VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include/)

# Source files
file(GLOB_RECURSE SOURCES main.cpp src/*.cpp)

# Executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Testing
enable_testing()
add_subdirectory(tests)
