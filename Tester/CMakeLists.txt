cmake_minimum_required(VERSION 3.22)
project(Probes)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Application/bin)

add_subdirectory(Joiner)
add_subdirectory(HLRouter)
add_subdirectory(LLRouter)
add_subdirectory(OptimizerApp)
