cmake_minimum_required(VERSION 3.30)

project(fsm VERSION 0.1 LANGUAGES CXX)

include(FetchContent)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
unset(ENV{VCPKG_ROOT} )
#set(CMAKE_TOOLCHAIN_FILE "C:/Users/ReWyn/^/projects/ICP/libs/vcpkg/scripts/buildsystems/vcpkg.cmake")


set(REPROC++ ON CACHE BOOL "Build the reproc C++ wrapper")
FetchContent_Declare(
        reproc
        GIT_REPOSITORY https://github.com/DaanDeMeyer/reproc.git
        GIT_TAG 3179928ae7b085e41dfb846d987519fa7c12ffb3    # v14.2.5
)
FetchContent_MakeAvailable(reproc)

set(BUILD_TESTING OFF)
set(ABSL_ENABLE_INSTALL ON)
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

FetchContent_Declare(
        range-v3
        GIT_REPOSITORY https://github.com/ericniebler/range-v3.git
        GIT_TAG a81477931a8aa2ad025c6bda0609f38e09e4d7ec
)
FetchContent_MakeAvailable(range-v3)

FetchContent_Declare(
        ctre
        GIT_REPOSITORY https://github.com/hanickadot/compile-time-regular-expressions.git
        GIT_TAG 78c5c3ef0e562f2ef0fea9ee3de743cdcf99e6f2    # v3.9.0
)
FetchContent_MakeAvailable(ctre)

FetchContent_Declare(
        fast_float
        GIT_REPOSITORY https://github.com/fastfloat/fast_float.git
        GIT_TAG tags/v8.0.2
        GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(fast_float)

add_executable(fsm
        fsm/AutomatLib.cpp
        fsm/parser.cpp
        fsm/Utils.cpp
        fsm/AutomatRuntime.cpp
        fsm/main.cpp
        fsm/fsmConstructed.template.cpp
        fsm/tof5s.template.cpp
        fsm/messages/server.cpp
        fsm/messages/server.h
        fsm/messages/client.cpp
        fsm/messages/client.h
)

find_package(cppzmq)
find_package(ZeroMQ CONFIG REQUIRED)

target_link_libraries(fsm PRIVATE
        reproc++
        ctre::ctre  # Considered obsolete
        absl::base
        absl::strings
        absl::log
        absl::log_initialize
        range-v3::range-v3
        re2::re2
        fast_float
        libzmq
        cppzmq
)

