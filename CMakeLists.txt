cmake_minimum_required(VERSION 3.50)
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


# core lib
file(GLOB_RECURSE LIB_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_library(etched_lib STATIC ${LIB_SOURCES})
target_include_directories(etched_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE etched_lib)


# Testing
enable_testing()
add_subdirectory(tests)
