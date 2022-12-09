cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
set(PROJECT_NAME abm)
project("${PROJECT_NAME}" VERSION 0.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# dependencies
find_package(ABM REQUIRED
  HINTS cmake
)

add_executable("${PROJECT_NAME}"
  src/main.cpp
)

message(STATUS "INCLUDE DIRS: ${ABMSDK_INCLUDE_DIRS}")
target_include_directories("${PROJECT_NAME}" BEFORE
  PUBLIC
    ABMSDK_INCLUDE_DIRS
)

install(TARGETS "${PROJECT_NAME}" DESTINATION bin)
