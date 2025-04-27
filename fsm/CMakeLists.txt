cmake_minimum_required(VERSION 3.16)

project(fsm VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_COMPILER gcc)

add_executable(fsm
  StateMachine.h
  StateMachine.cpp)