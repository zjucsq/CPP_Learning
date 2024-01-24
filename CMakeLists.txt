cmake_minimum_required(VERSION 3.22)
project(cpp20)

# set (CMAKE_C_COMPILER "/usr/bin/gcc-9")
# set (CMAKE_CXX_COMPILER "/usr/bin/g++-9")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # For clang-tidy.
set(BUILD_SHARED_LIBS OFF)              # We expect external libraries to be linked statically.
set(CMAKE_CXX_STANDARD 20)              # Compile as C++20.
set(CMAKE_CXX_STANDARD_REQUIRED ON)     # Require C++20 support.

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -pthread")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB CPP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*/*/*.cpp")

set(NOT_COMPILE "generator" "parser" "variant" "variant_example" "wordcount")

foreach(file ${CPP_FILES})
    get_filename_component(filename ${file} NAME_WE)
    if (NOT ${filename} IN_LIST NOT_COMPILE)
        add_executable(${filename} ${file})
        message("add ${filename}")
    endif()
endforeach()

