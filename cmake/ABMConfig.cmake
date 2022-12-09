# ABM SDK environment variable must be set

if(DEFINED ENV{ABMSDK})
  set(ABMSDK_FOUND true)
endif()

set(ABMSDK_INCLUDE_DIRS "$ENV{ABMSDK}/include")
set(ABMSDK_LIBRARY_DIRS "$ENV{ABMSDK}/lib")

file(GLOB ABMSDK_LIBRARIES
  RELATIVE "${ABMSDK_LIBRARY_DIRS}"
  "${ABMSDK_LIBRARY_DIRS}/*.lib"
)

if(NOT EXISTS "${ABMSDK_INCLUDE_DIRS}/Athena.h")
  message(FATAL_ERROR "Could not find athena file")
endif()
