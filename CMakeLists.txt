cmake_minimum_required(VERSION 3.14)

project(AssetBuilder VERSION 1.0.0 LANGUAGES CXX)

include(cmake/dependencies.cmake)
include(cmake/ProjectConfig.cmake)

add_subdirectory(Common)
add_subdirectory(AssetBuilder)
