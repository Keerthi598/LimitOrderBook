cmake_minimum_required(VERSION 3.12)
project(HighFrequency VERSION 1.0)

# 1. Global Settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/release)

if(MSVC)
    add_compile_options(/O2 /Ot /Oi /Gy) # MSVC Release optimizations
else()
    add_compile_options(-O3 -march=native -flto) # GCC/Clang: Max speed + Link Time Optimization
endif()

add_library(
        ipc_protocol
        INTERFACE
)

target_include_directories(
        ipc_protocol
        INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(
        ipc_protocol
        INTERFACE
            cxx_std_20
)

add_definitions(-DDATA_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/historical")

# Add Subdirectories
add_subdirectory(src)

# Create the Tests
# add_subdirectory(tests)
