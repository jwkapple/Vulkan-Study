# CMake version
cmake_minimum_required(VERSION 3.17.2 FATAL_ERROR)

# Declare project
project(Vulkan-Study)

# Set ouput directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

# Excutables
add_executable(Vulkan-Study src/main.cpp)