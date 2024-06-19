cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
include(FetchContent)
if(DEFINED PROJECT_NAME)
    set(PARENT_PROJECT "${PROJECT_NAME}")
endif(DEFINED PROJECT_NAME)
set(PROJECT_NAME packet)
project("${PROJECT_NAME}" VERSION 0.1 LANGUAGES CXX C)

# CMake requirements
set(CMAKE_CXX_STANDARD 20)  # Standard C++ Library 2020
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Flags
if(UNIX AND NOT APPLE)
	set(LINUX ON)
endif()
if(APPLE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacos-version-min=10.15")
elseif(LINUX)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

# Packate C++ Library
add_library("${PROJECT_NAME}"
    src/packet.cpp
)
target_include_directories("${PROJECT_NAME}" BEFORE
    PUBLIC
        "include"
)

if(NOT DEFINED pybind11_SOURCE_DIR)
    # PYBIND11
    FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11
    GIT_TAG master
    )
    FetchContent_MakeAvailable(pybind11)
    add_subdirectory("${pybind11_SOURCE_DIR}")
endif(NOT DEFINED pybind11_SOURCE_DIR)

# Build the python module
pybind11_add_module("_${PROJECT_NAME}"
    src/module.cpp
)

target_include_directories("_${PROJECT_NAME}" BEFORE
    PUBLIC
        include
)

target_link_libraries("_${PROJECT_NAME}"
    PUBLIC
        "${PROJECT_NAME}"
)

install(TARGETS "_${PROJECT_NAME}" DESTINATION abmbci)
if(DEFINED PARENT_PROJECT)
    set(PROJECT_NAME "${PARENT_PROJECT}")
endif(DEFINED PARENT_PROJECT)