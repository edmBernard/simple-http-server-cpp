project(uws)

#**************************************************************************************************
# General cMake settings
#**************************************************************************************************
cmake_minimum_required(VERSION 3.5)

#**************************************************************************************************
# Find Package **************************************************************************************************
find_library(USOCKETS_LIB uSockets)
find_package(ZLIB REQUIRED)
find_package(cxxopts REQUIRED)

#**************************************************************************************************
# Include **************************************************************************************************
include_directories(${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# get_cmake_property(_variableNames VARIABLES)
# list (SORT _variableNames)
# foreach (_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()

#**************************************************************************************************
# Set variable **************************************************************************************************
SET(SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)


#**************************************************************************************************
# Set compiler **************************************************************************************************
SET(CMAKE_CXX_FLAGS "-std=c++17 -Wall -O3 -fPIC -fpermissive")


#**************************************************************************************************
# Make configuration
#**************************************************************************************************
add_executable(uws ${SOURCES})
target_link_libraries(uws -pthread -lstdc++fs ${USOCKETS_LIB} ZLIB::ZLIB)
