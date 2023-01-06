# ABM SDK environment variable must be set

if(DEFINED ENV{ABMSDK})
  set(ABMSDK_FOUND true)
else()
  set(ENV{ABMSDK} "C:\\ABM\\B-Alert\\SDK")
endif()

set(ABMSDK_INCLUDE_DIRS "$ENV{ABMSDK}/include")
set(ABMSDK_LIBRARY_DIRS "$ENV{ABMSDK}/lib")
set(ABMSDK_BINARY_DIRS "$ENV{ABMSDK}/bin")

file(GLOB ABMSDK_LIBRARIES
  "${ABMSDK_LIBRARY_DIRS}/*.lib"
)
file(GLOB ABMSDK_BINARIES
  "${ABMSDK_LIBRARY_DIRS}/*.dll"
)

if(NOT EXISTS "${ABMSDK_INCLUDE_DIRS}/Athena.h")
  message(FATAL_ERROR "Could not find athena file")
endif()

install(DIRECTORY "${ABMSDK_BINARY_DIRS}/"
  DESTINATION "${CMAKE_BINARY_DIR}/bin"
)

if(MINGW)
  set(LIBEXT ".a")
endif()

add_library(ABM_athena SHARED IMPORTED)
set_property(TARGET ABM_athena PROPERTY IMPORTED_LOCATION "${ABMSDK_BINARY_DIRS}/ABM_Athena.dll")
set_property(TARGET ABM_athena PROPERTY IMPORTED_IMPLIB "${ABMSDK_LIBRARY_DIRS}/ABM_Athena.lib${LIBEXT}")
