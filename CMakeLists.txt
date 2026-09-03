cmake_minimum_required(VERSION 3.15)

project(ApiHashing CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Исправили add -> add_executable
add_executable(app
    src/main.cpp)