# Set the minimum version of CMake required
cmake_minimum_required(VERSION 3.10)

# Extract source file name (without extension) to use as project name
get_filename_component(PROJECT_NAME ${CMAKE_SOURCE_DIR}/src/test.cpp NAME_WE)

# Set the project name dynamically from the source file name
project(${PROJECT_NAME})

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)

# Specify the executable to build
add_executable(${PROJECT_NAME} src/test.cpp)
