cmake_minimum_required(VERSION 3.5)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

project(heatsink LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory("${CMAKE_SOURCE_DIR}/src")

enable_testing(true)
add_subdirectory("${CMAKE_SOURCE_DIR}/test")
