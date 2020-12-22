cmake_minimum_required(VERSION 3.17)
project(file_system_c99 C)

set(CMAKE_C_STANDARD 99)

aux_source_directory(. DIR_SRC)

add_executable(file_system_c99 ${DIR_SRC})
