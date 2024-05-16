cmake_minimum_required(VERSION 3.22)
project(HLRouter)

set(CMAKE_CXX_STANDARD 20)

add_executable(LLRouter
        main.cpp
        App.cpp
        ../Common/Math.cpp
        ../Common/Utils.cpp
        Board/Board.cpp
        Board/Probe.cpp
        Board/Zones.cpp
        Collisions/Collision.cpp
)
