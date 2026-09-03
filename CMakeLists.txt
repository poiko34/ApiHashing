cmake_minimum_required(VERSION 3.15)

project(ApiHashing CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(app
    src/main.cpp
    src/ApiHashing.cpp)

target_compile_options(app PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/Os>
    
    $<$<CXX_COMPILER_ID:GNU>:-Os>
    
    $<$<CXX_COMPILER_ID:Clang>:-Oz>
)