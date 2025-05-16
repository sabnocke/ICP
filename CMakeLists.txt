include(FetchContent)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")
file(COPY ${CMAKE_SOURCE_DIR}/stdlib.lua DESTINATION ${CMAKE_BINARY_DIR})

cmake_minimum_required(VERSION 3.11)
project(fsm VERSION 0.1 LANGUAGES CXX)


set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
unset(ENV{VCPKG_ROOT})



#FetchContent_Declare(
#        fast_float
#        GIT_REPOSITORY https://github.com/fastfloat/fast_float.git
#        GIT_TAG tags/v8.0.2
#        GIT_SHALLOW TRUE)
#FetchContent_MakeAvailable(fast_float)

add_executable(fsm
        fsm/AutomatLib.cpp
        fsm/ParserLib.cpp
        fsm/Utils.cpp
        fsm/main.cpp
        fsm/types/transitions.cpp
        fsm/types/variables.cpp
        fsm/types/states.cpp
        fsm/Interpret.cpp
)

find_package(Lua REQUIRED)
find_package(absl CONFIG REQUIRED)
find_package(re2 CONFIG REQUIRED)
find_package(range-v3 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
#
if (TARGET Lua::lua)
    target_link_libraries(fsm PRIVATE Lua::lua)
else ()
    # Fallback for older FindLua.cmake modules that don't define Lua::lua
    if (LUA_INCLUDE_DIR)
        target_include_directories(fsm PRIVATE ${LUA_INCLUDE_DIR})
    endif ()
    if (LUA_LIBRARIES)
        target_link_libraries(fsm PRIVATE ${LUA_LIBRARIES})
    else ()
        message(FATAL_ERROR "Lua was found (LUA_FOUND=${LUA_FOUND}) but neither Lua::lua target nor LUA_LIBRARIES variable were defined. Cannot link Lua.")
    endif ()
endif ()

target_link_libraries(fsm PRIVATE
        range-v3::range-v3
        re2::re2
#        fast_float
        absl::btree
        absl::container_common
        absl::log
        absl::log_initialize
        absl::log_flags
        absl::flags
        spdlog::spdlog_header_only
)

# add_subdirectory(src/icp-qt)