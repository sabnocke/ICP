cmake_minimum_required(VERSION 3.16)

project(fsm VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_COMPILER gcc)
set(Boost_NO_SYSTEM_PATHS ON)

add_subdirectory(libs/compile-time-regular-expressions)

add_executable(fsm
        fsm/StateMachine.h
        fsm/StateMachine.cpp
        fsm/AutomatLib.cpp
        fsm/AutomatLib.h
        fsm/Stopwatch.cpp
        fsm/Stopwatch.h
        fsm/parser.cpp
        fsm/parser.h
)


target_link_libraries(fsm ctre::ctre)
