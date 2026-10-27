cmake_minimum_required(VERSION 3.15)
project(WinOpacity LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 生成 GUI 程序（不弹控制台窗口）。
if(WIN32)
    add_executable(WinOpacity WIN32
        src/main.cpp
        src/config.cpp
        src/transparency_controller.cpp
        src/window_picker.cpp
        src/input_handler.cpp
    )
else()
    add_executable(WinOpacity
        src/main.cpp
        src/config.cpp
        src/transparency_controller.cpp
        src/window_picker.cpp
        src/input_handler.cpp
    )
endif()

if(MSVC)
    target_compile_options(WinOpacity PRIVATE /W4 /utf-8)
else()
    target_compile_options(WinOpacity PRIVATE -Wall -Wextra)
endif()
