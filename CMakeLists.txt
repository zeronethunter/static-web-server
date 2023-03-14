cmake_minimum_required(VERSION 3.22.1)
project(static_web_server VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CLANG_WARNINGS
        -Wall
        -Werror
        -Wextra
        -Wshadow

        -Wcast-align
        -Wunused
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
        )

link_directories(/usr/local/lib)

set(SERVER_INCLUDE include)

include_directories(${SERVER_INCLUDE})

file(GLOB_RECURSE FILES
        "${PROJECT_SOURCE_DIR}/include/*.h"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        )

add_executable(static_web_server main.cpp ${FILES})

target_link_libraries(static_web_server PUBLIC event event_pthreads pthread)
