cmake_minimum_required(VERSION 4.1)
project(algo)

set(CMAKE_CXX_STANDARD 20)

set(DATA_PATH "${CMAKE_SOURCE_DIR}/tmp/")
if(NOT EXISTS "${DATA_PATH}")
    file(MAKE_DIRECTORY "${DATA_PATH}")
endif()
add_compile_definitions(DATA_DIR="${DATA_PATH}")
add_subdirectory(lab2)
