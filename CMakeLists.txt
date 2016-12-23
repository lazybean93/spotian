# Specify the minimum version for CMake

cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_COMPILER g++-4.9)
# Project's name

project(spotian)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
# Set the output folder where your program will be created
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
# The following folder will be included
include_directories( /usr/include /usr/include/dbus-1.0 /usr/lib/x86_64-linux-gnu/dbus-1.0/include "${PROJECT_SOURCE_DIR}")

add_library(Metadata ${PROJECT_SOURCE_DIR}/Metadata.cpp)
add_library(Player ${PROJECT_SOURCE_DIR}/Player.cpp)
add_library(Playlist ${PROJECT_SOURCE_DIR}/Playlist.cpp)
add_library(Recorder ${PROJECT_SOURCE_DIR}/Recorder.cpp)
add_library(Timeout ${PROJECT_SOURCE_DIR}/timeout.cpp)
add_library(Tools ${PROJECT_SOURCE_DIR}/tools.cpp)
add_executable(Spotian ${PROJECT_SOURCE_DIR}/spotian.cpp)
#link_directories(${PROJECT_SOURCE_DIR} ${CMAKE_BINARY_DIR})
find_library(LIBBASS bass ./)
target_link_libraries(Spotian Metadata Player Playlist Recorder Timeout Tools ${LIBBASS} dbus-1)

