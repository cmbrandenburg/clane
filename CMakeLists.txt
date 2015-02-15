cmake_minimum_required(VERSION 3.0.2)
project(clane LANGUAGES C CXX)
enable_testing()
find_package(Boost 1.55 REQUIRED system)
find_package(Doxygen)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

find_program(MEMORYCHECK_COMMAND valgrind)
set(MEMORYCHECK_COMMAND_OPTIONS "--track-children=yes --leak-check=full")

add_subdirectory(src)
add_subdirectory(doc)
