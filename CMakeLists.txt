cmake_minimum_required(VERSION 3.11)


set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(fsm VERSION 0.1 LANGUAGES CXX)

include(FetchContent)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
unset(ENV{VCPKG_ROOT} )



#set(REPROC++ ON CACHE BOOL "Build the reproc C++ wrapper")
#FetchContent_Declare(
#        reproc
#        GIT_REPOSITORY https://github.com/DaanDeMeyer/reproc.git
#        GIT_TAG 3179928ae7b085e41dfb846d987519fa7c12ffb3    # v14.2.5
#)
#FetchContent_MakeAvailable(reproc)

#FetchContent_Declare(
#        rxcpp
#        GIT_REPOSITORY https://github.com/ReactiveX/RxCpp.git
#        GIT_TAG tags/v4.0.0
#)
#FetchContent_MakeAvailable(rxcpp)

find_package(absl CONFIG REQUIRED)
find_package(re2 CONFIG REQUIRED)

#! vcpkg can provide this
FetchContent_Declare(
        range-v3
        GIT_REPOSITORY https://github.com/ericniebler/range-v3.git
        GIT_TAG a81477931a8aa2ad025c6bda0609f38e09e4d7ec
)
FetchContent_MakeAvailable(range-v3)
#
FetchContent_Declare(
        fast_float
        GIT_REPOSITORY https://github.com/fastfloat/fast_float.git
        GIT_TAG tags/v8.0.2
        GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(fast_float)

#FetchContent_Declare(
#        luacpp
#        GIT_REPOSITORY https://github.com/jordanvrtanoski/luacpp.git
#        GIT_TAG d7171e83dbcab9b52adafd1c939f4f2c6ed861b1
#)
#FetchContent_MakeAvailable()

add_executable(fsm
        fsm/AutomatLib.cpp
        fsm/ParserLib.cpp
        fsm/Utils.cpp
        fsm/main.cpp
        fsm/types/transitions.cpp
        fsm/types/variables.cpp
        fsm/types/states.cpp
#        fsm/Timing.cpp
        #        fsm/Interpret.cpp
)

# find_package(cppzmq)
#find_package(Lua REQUIRED)
#
#if(TARGET Lua::lua)
#    target_link_libraries(fsm PRIVATE Lua::lua)
#else()
#    # Fallback for older FindLua.cmake modules that don't define Lua::lua
#    if(LUA_INCLUDE_DIR)
#        target_include_directories(fsm PRIVATE ${LUA_INCLUDE_DIR})
#    endif()
#    if(LUA_LIBRARIES)
#        target_link_libraries(fsm PRIVATE ${LUA_LIBRARIES})
#    else()
#        message(FATAL_ERROR "Lua was found (LUA_FOUND=${LUA_FOUND}) but neither Lua::lua target nor LUA_LIBRARIES variable were defined. Cannot link Lua.")
#    endif()
#endif()

#target_include_directories(fsm PRIVATE ${LUA_INCLUDE_DIR})
#find_package(rxcpp CONFIG)
# find_package(ZeroMQ CONFIG REQUIRED)

#add_subdirectory(./libs/range-v3)

target_link_libraries(fsm PRIVATE
#        reproc++
        range-v3::range-v3
        re2::re2
        fast_float
        absl::btree
        absl::container_common
        absl::log
        absl::log_initialize
#        rxcpp
##        libzmq-static
##        cppzmq-static
)

