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

# 所有产物统一输出到 build/bin（VS 多配置时为 bin/Debug），
# 保证 exe 与自己的 DLL 同目录，避免 DLL 搜索失败（0xC0000135）
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# MSVC 源文件按 UTF-8 解析（避免 C4819 中文注释警告）
if(MSVC)
    add_compile_options(/utf-8)
endif()

find_package(Qt5 COMPONENTS Core Widgets Network REQUIRED)

# 渲染模块开关：当前暂不实现渲染，默认关闭（无需 VTK）
# 打开方式：cmake -DIDOS_ENABLE_RENDER=ON --preset win-x64-debug-local
option(IDOS_ENABLE_RENDER "Enable VTK rendering module" OFF)

if(IDOS_ENABLE_RENDER)
    find_package(VTK REQUIRED COMPONENTS
        CommonCore
        CommonDataModel
        RenderingCore
        RenderingOpenGL2
        InteractionStyle
        GUISupportQt
    )
endif()

# 源码按模块组织
add_subdirectory(src)
add_subdirectory(resources)
add_subdirectory(i18n)

# ---------------------------------------------------------------------
# include 架构边界守护（最严格模式：每次构建自动执行，违规即构建失败）
# 规则明细见 tools/check_includes.ps1 头部注释
# ---------------------------------------------------------------------
set(IDOS_CHECK_INCLUDES_SCRIPT "${CMAKE_SOURCE_DIR}/tools/check_includes.ps1")

add_custom_target(check_includes ALL
    COMMAND powershell -NoProfile -ExecutionPolicy Bypass
            -File "${IDOS_CHECK_INCLUDES_SCRIPT}"
            -SourceRoot "${CMAKE_SOURCE_DIR}/src"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMENT "检查 include 架构边界..."
    VERBATIM
)

# 同时纳入 CTest（CI / 手动体检：ctest -R check_includes）
enable_testing()
add_test(NAME check_includes
    COMMAND powershell -NoProfile -ExecutionPolicy Bypass
            -File "${IDOS_CHECK_INCLUDES_SCRIPT}"
            -SourceRoot "${CMAKE_SOURCE_DIR}/src"
)