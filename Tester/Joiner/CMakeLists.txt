cmake_minimum_required(VERSION 3.22)
project(HLRouter)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenMP)

add_library(Joiner SHARED
        Board.cpp
        Board.h
        ../Interfaces/IBoard.h
        ../Common/Math.cpp
        ../Common/Math.h
        ../Common/Utils.cpp
        ../Common/Utils.h)


if (OPENMP_FOUND)
    target_link_libraries(Joiner PUBLIC OpenMP::OpenMP_CXX)
endif()

set_target_properties(Joiner PROPERTIES PUBLIC_HEADER Board.h)