cmake_minimum_required(VERSION 3.16)

project(fsm VERSION 0.1 LANGUAGES CXX)

include(FetchContent)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(REPROC++ ON CACHE BOOL "Build the reproc C++ wrapper")
set(BUILD_TESTING OFF)
set(ABSL_ENABLE_INSTALL ON)

FetchContent_Declare(
        reproc
        GIT_REPOSITORY https://github.com/DaanDeMeyer/reproc.git
        GIT_TAG 3179928ae7b085e41dfb846d987519fa7c12ffb3    # v14.2.5
)
FetchContent_MakeAvailable(reproc)

FetchContent_Declare(
        ctre
        GIT_REPOSITORY https://github.com/hanickadot/compile-time-regular-expressions.git
        GIT_TAG 78c5c3ef0e562f2ef0fea9ee3de743cdcf99e6f2    # v3.9.0
)
FetchContent_MakeAvailable(ctre)

FetchContent_Declare(
        absl
        GIT_REPOSITORY "https://github.com/abseil/abseil-cpp.git"
        GIT_TAG d9e4955c65cd4367dd6bf46f4ccb8cd3d100540b    # LTS 20250127.1
)
FetchContent_MakeAvailable(absl)

FetchContent_Declare(
        re2
        GIT_REPOSITORY https://github.com/google/re2.git
        GIT_TAG cf8f19116192016936b306b033f9860cff6f0b5c    # 2024-07-02
)
FetchContent_MakeAvailable(re2)

add_executable(fsm
        fsm/StateMachine.h
        fsm/StateMachine.cpp
        fsm/AutomatLib.cpp
        fsm/AutomatLib.h
        fsm/Stopwatch.cpp
        fsm/Stopwatch.h
        fsm/parser.cpp
        fsm/parser.h
        fsm/Utils.cpp
        fsm/Utils.h
)


target_link_libraries(fsm PRIVATE
        reproc++
        ctre::ctre
        absl::base
        re2::re2)

