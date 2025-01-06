cmake_minimum_required(VERSION 3.10)
project(baseline)

set(CMAKE_CXX_STANDARD 14)

# Define the macro ‘DEBUG' in the debug mode
if(CMAKE_BUILD_TYPE STREQUAL Debug)        
    ADD_DEFINITIONS(-DDEBUG)               
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "/O2 /arch:AVX2 /W1 /EHsc")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(CMAKE_CXX_FLAGS "-O3 -xHost")
else()
    # clang and gcc
    set(CMAKE_CXX_FLAGS "-O3 -march=native -w")
endif()

include_directories(src/core)
add_executable(graph src/graph/main.cpp)

set(DOCTEST_DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/doctest)
file(DOWNLOAD
    https://raw.githubusercontent.com/onqtam/doctest/2.4.6/doctest/doctest.h
    ${DOCTEST_DOWNLOAD_DIR}/doctest.h
    EXPECTED_HASH SHA1=40728d2bed7f074e80cb095844820114e7d16ce0
)
