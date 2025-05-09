cmake_minimum_required(VERSION 3.30)


set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/libs/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")


project(fsm VERSION 0.1 LANGUAGES CXX)

include(FetchContent)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
unset(ENV{VCPKG_ROOT} )



set(REPROC++ ON CACHE BOOL "Build the reproc C++ wrapper")
FetchContent_Declare(
        reproc
        GIT_REPOSITORY https://github.com/DaanDeMeyer/reproc.git
        GIT_TAG 3179928ae7b085e41dfb846d987519fa7c12ffb3    # v14.2.5
)
FetchContent_MakeAvailable(reproc)

#set(BUILD_TESTING OFF)
#set(ABSL_ENABLE_INSTALL ON)
#FetchContent_Declare(
#        absl
#        GIT_REPOSITORY "https://github.com/abseil/abseil-cpp.git"
#        GIT_TAG d9e4955c65cd4367dd6bf46f4ccb8cd3d100540b    # LTS 20250127.1
#)
#FetchContent_MakeAvailable(absl)

find_package(absl CONFIG REQUIRED)
find_package(re2 CONFIG REQUIRED)

#FetchContent_Declare(
#        re2
#        GIT_REPOSITORY https://github.com/google/re2.git
#        GIT_TAG cf8f19116192016936b306b033f9860cff6f0b5c    # 2024-07-02
#)
#FetchContent_MakeAvailable(re2)

#FetchContent_Declare(
#        range-v3
#        GIT_REPOSITORY https://github.com/ericniebler/range-v3.git
#        GIT_TAG a81477931a8aa2ad025c6bda0609f38e09e4d7ec
#)
#FetchContent_MakeAvailable(range-v3)

FetchContent_Declare(
        fast_float
        GIT_REPOSITORY https://github.com/fastfloat/fast_float.git
        GIT_TAG tags/v8.0.2
        GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(fast_float)


add_executable(fsm
        fsm/AutomatLib.cpp
        fsm/ParserLib.cpp
        fsm/Utils.cpp
        fsm/AutomatRuntime.cpp
        fsm/main.cpp
        fsm/views.h
        fsm/types/transitions.cpp
        fsm/types/transitions.h
        fsm/types/variables.cpp
        fsm/types/transitions.h
        fsm/types/all_types.h
        fsm/types/states.cpp
        fsm/types/states.h
        fsm/timer.cpp
        fsm/timer.h
        #        fsm/fsmConstructed.template.cpp
#        fsm/tof5s.template.cpp
#        fsm/messages/p2p.cpp
)

find_package(cppzmq)
find_package(rxcpp CONFIG)
# find_package(ZeroMQ CONFIG REQUIRED)

add_subdirectory(./libs/range-v3)

target_link_libraries(fsm PRIVATE
        reproc++
        range-v3::range-v3
        re2::re2
        fast_float
        absl::btree
        absl::container_common
        absl::log
        absl::log_initialize
        rxcpp
#        libzmq-static
#        cppzmq-static
)

