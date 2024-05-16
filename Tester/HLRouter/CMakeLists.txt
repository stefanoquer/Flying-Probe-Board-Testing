cmake_minimum_required(VERSION 3.22)
project(HLRouter)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenMP)

add_library(HLRouter SHARED
        ../Common/Math.cpp
        ../Common/Utils.cpp
        Board.cpp
)

if (OPENMP_FOUND)
    target_link_libraries(HLRouter PUBLIC OpenMP::OpenMP_CXX)
endif()

set_target_properties(HLRouter PROPERTIES PUBLIC_HEADER Board.h)
