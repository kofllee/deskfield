cmake_minimum_required(VERSION 3.24)

project(Deskfield LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Deskfield
        src/main.cpp
        src/windowing/WindowEnumerator.cpp
)

target_include_directories(Deskfield PRIVATE
        src
)

target_compile_definitions(Deskfield PRIVATE
        UNICODE
        _UNICODE
        WIN32_LEAN_AND_MEAN
)

target_link_libraries(Deskfield PRIVATE
        user32
)