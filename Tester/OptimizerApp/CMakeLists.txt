cmake_minimum_required(VERSION 3.22)
project(OptimizerApp)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenMP)

add_executable(OptimizerApp
        main.cpp
        App.cpp
        )

target_link_libraries(OptimizerApp HLRouter Joiner)
