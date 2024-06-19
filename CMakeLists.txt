cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
include(FetchContent)
set(PROJECT_NAME abmbci)
project("${PROJECT_NAME}" VERSION 0.1 LANGUAGES CXX C)

# CMake requirements
set(CMAKE_CXX_STANDARD 20)  # Standard C++ Library 2020
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

if(DEFINED BUILDMAIN)
  # dependencies
  find_package(ABM REQUIRED
    HINTS cmake
  )
  
  # Define the main executable
  add_executable("${PROJECT_NAME}" WIN32
    src/sdk/logging.cpp
    src/headset/abmheadset.cpp
    src/main.cpp
  )
  if(MSVC)
      target_compile_options("${PROJECT_NAME}" PUBLIC /Gd)
  endif()
  message(STATUS "INCLUDE DIRS: ${ABMSDK_INCLUDE_DIRS}")
  target_include_directories("${PROJECT_NAME}" BEFORE
    PUBLIC
      "${ABMSDK_INCLUDE_DIRS}"
      "include"
  )

  target_link_libraries("${PROJECT_NAME}"
    PRIVATE
      ABM_athena
  )

  install(TARGETS "${PROJECT_NAME}" DESTINATION "${CMAKE_BINARY_DIR}/bin")
  return()
endif(DEFINED BUILDMAIN)

# Flags
if(UNIX AND NOT APPLE)
	set(LINUX ON)
endif()
if(APPLE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacos-version-min=10.15")
elseif(LINUX)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()


# PYBIND11
FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11
  GIT_TAG master
)
FetchContent_MakeAvailable(pybind11)
add_subdirectory("${pybind11_SOURCE_DIR}")

add_subdirectory("packet")

if(MSVC)
  # Build the python module
  pybind11_add_module("_${PROJECT_NAME}"
    src/headset/abmheadset.cpp
    src/sdk/callbacks.cpp
    src/sdk/device_info.cpp
    src/sdk/eeg_channel_info.cpp
    src/sdk/logging.cpp
    src/abmbci.cpp
  )

  set(ABMSDK "$ENV{ABMSDK}")
  cmake_path(SET ABMSDK_PATH "${ABMSDK}")
  cmake_path(GET ABMSDK PARENT_PATH SYSTEM_PATH)
  cmake_path(APPEND SYSTEM_PATH "Config" OUTPUT_VARIABLE CONFIG_PATH)
  cmake_path(APPEND ABMSDK_PATH "lib" OUTPUT_VARIABLE SDKLIB_PATH)
  cmake_path(APPEND ABMSDK_PATH "include" OUTPUT_VARIABLE SDKINC_PATH)

  message(STATUS "Include path: " "${SDKINC_PATH}")
  target_include_directories("_${PROJECT_NAME}" BEFORE
    PUBLIC
      "${SDKINC_PATH}"
      include
      packet/include
  )

  string(REPLACE "/" "\\\\" ABMSDK_DIR "${ABMSDK_PATH}")
  string(REPLACE "/" "\\\\" CONFIG_DIR "${CONFIG_PATH}")
  string(CONCAT ABMSDK_MACRO "L\"${ABMSDK_DIR}\"")
  string(CONCAT CONFIG_MACRO "L\"${CONFIG_DIR}\"")
  message(STATUS "PATH: " "${ABMSDK_MACRO}")
  add_compile_definitions(__PYBIND11__="1" __ABMSDK__=${ABMSDK_MACRO} __CONFIG__=${CONFIG_MACRO})

  if(MSVC)
    add_compile_options(/DWIN32 /D_WINDOWS /DEBUG)
    add_link_options(/DEBUG)
  endif(MSVC)

  target_link_directories("_${PROJECT_NAME}"
    PUBLIC "${SDKLIB_PATH}"
  )

  target_link_libraries("_${PROJECT_NAME}"
    PUBLIC
      "ABM_Athena"
      packet
  )
  install(TARGETS "_${PROJECT_NAME}" DESTINATION abmbci)
endif(MSVC)