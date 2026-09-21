cmake_minimum_required(VERSION 3.16)

project(IDOS VERSION 1.0.0 LANGUAGES CXX C)

set(IDOS_QT_DIR ""
    CACHE PATH "Qt5 install root (contains lib/cmake/Qt5)")

set(VTK_DIR "${CMAKE_SOURCE_DIR}/thirdparty/vtk9.6.0/lib/cmake/vtk-9.6"
    CACHE PATH "VTK cmake config dir")

if(IDOS_QT_DIR STREQUAL "")
    message(FATAL_ERROR
        "IDOS_QT_DIR is not set.\n"
        "How to fix:\n"
        "  1. Copy CMakeUserPresets.example.json to CMakeUserPresets.json\n"
        "  2. Edit IDOS_QT_DIR to your Qt5 install path\n"
        "  3. Or: cmake -DIDOS_QT_DIR=xxx --preset win-x64-debug"
    )
endif()

set(CMAKE_PREFIX_PATH "${IDOS_QT_DIR}")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Debug 产物加 d 后缀：idos_cored.dll / idos_renderd.dll / idosd.exe
set(CMAKE_DEBUG_POSTFIX "d")

# MSVC 源文件按 UTF-8 解析（避免 C4819 中文注释警告）
if(MSVC)
    add_compile_options(/utf-8)
endif()

find_package(Qt5 COMPONENTS Core Widgets Network REQUIRED)

find_package(VTK REQUIRED COMPONENTS
    CommonCore
    CommonDataModel
    RenderingCore
    RenderingOpenGL2
    InteractionStyle
    GUISupportQt
)

# 源码按模块组织
add_subdirectory(src)
add_subdirectory(resources)
add_subdirectory(i18n)