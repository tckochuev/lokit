cmake_minimum_required(VERSION 3.0.2)
project(lokit CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(APPLE)
    SET(CMAKE_OSX_ARCHITECTURES "x86_64")
endif()

add_subdirectory(src)
