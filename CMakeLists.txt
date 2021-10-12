cmake_minimum_required(VERSION 3.21)

#**************************************************************************************************
# Project *****************************************************************************************
project(simple-http-server
        HOMEPAGE_URL https://github.com/edmBernard/simple-http-server-cpp
        LANGUAGES CXX)

#**************************************************************************************************
# Find Package ************************************************************************************
find_library(USOCKETS_LIB uSockets)
find_package(ZLIB REQUIRED)
find_package(cxxopts REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

find_package(unofficial-libuv CONFIG REQUIRED)

#**************************************************************************************************
# Include *****************************************************************************************
include_directories(${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# get_cmake_property(_variableNames VARIABLES)
# list (SORT _variableNames)
# foreach (_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()

#**************************************************************************************************
# Set compiler Flags ******************************************************************************
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  add_compile_options(/Oy /utf-8)
else()
  add_compile_options(-fPIC -O3 -fno-omit-frame-pointer)
endif()

#**************************************************************************************************
# Set variable ************************************************************************************
SET(SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

#**************************************************************************************************
# Make configuration ******************************************************************************
add_executable(simple-http-server ${SOURCES})
target_link_libraries(simple-http-server PRIVATE -pthread -lstdc++fs ${USOCKETS_LIB} ZLIB::ZLIB fmt::fmt spdlog::spdlog)
