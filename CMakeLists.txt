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

# ---------------------------------------------------------------------
# SARibbon 第三方库（仅 include/lib/bin 三段式布局，无 CMake config 文件）
# 通过 IMPORTED target 暴露：include 路径 + 链接库 + Debug/Release 自动切换
# Debug 产物 SARibbonBard.{dll,lib}，Release 产物 SARibbonBar.{dll,lib}
# ---------------------------------------------------------------------
add_library(SARibbon::SARibbonBar SHARED IMPORTED)
set_target_properties(SARibbon::SARibbonBar PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/thirdparty/SARibbon/include"
    # VS 多配置构建按配置自动选库；IMPORTED_LOCATION 为 fallback（命令行单配置）
    IMPORTED_LOCATION_DEBUG   "${CMAKE_SOURCE_DIR}/thirdparty/SARibbon/bin/SARibbonBard.dll"
    IMPORTED_IMPLIB_DEBUG      "${CMAKE_SOURCE_DIR}/thirdparty/SARibbon/lib/SARibbonBard.lib"
    IMPORTED_LOCATION_RELEASE  "${CMAKE_SOURCE_DIR}/thirdparty/SARibbon/bin/SARibbonBar.dll"
    IMPORTED_IMPLIB_RELEASE    "${CMAKE_SOURCE_DIR}/thirdparty/SARibbon/lib/SARibbonBar.lib"
    IMPORTED_LOCATION          "${CMAKE_SOURCE_DIR}/thirdparty/SARibbon/bin/SARibbonBar.dll"
    IMPORTED_IMPLIB            "${CMAKE_SOURCE_DIR}/thirdparty/SARibbon/lib/SARibbonBar.lib"
)

# ---------------------------------------------------------------------
# Qt Advanced Docking System（预编译三段式：include/lib/bin，无 CMake config）
# 通过 IMPORTED target 暴露：include 路径 + 链接库 + Debug/Release 自动切换
# Debug: qtadvanceddocking-qt5d.{dll,lib}，Release: qtadvanceddocking-qt5.{dll,lib}
# ---------------------------------------------------------------------
add_library(ADS::ADS SHARED IMPORTED)
set_target_properties(ADS::ADS PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/thirdparty/AdvancedDockingSystem/include"
    IMPORTED_LOCATION_DEBUG   "${CMAKE_SOURCE_DIR}/thirdparty/AdvancedDockingSystem/bin/qtadvanceddocking-qt5d.dll"
    IMPORTED_IMPLIB_DEBUG      "${CMAKE_SOURCE_DIR}/thirdparty/AdvancedDockingSystem/lib/qtadvanceddocking-qt5d.lib"
    IMPORTED_LOCATION_RELEASE  "${CMAKE_SOURCE_DIR}/thirdparty/AdvancedDockingSystem/bin/qtadvanceddocking-qt5.dll"
    IMPORTED_IMPLIB_RELEASE    "${CMAKE_SOURCE_DIR}/thirdparty/AdvancedDockingSystem/lib/qtadvanceddocking-qt5.lib"
    IMPORTED_LOCATION          "${CMAKE_SOURCE_DIR}/thirdparty/AdvancedDockingSystem/bin/qtadvanceddocking-qt5.dll"
    IMPORTED_IMPLIB            "${CMAKE_SOURCE_DIR}/thirdparty/AdvancedDockingSystem/lib/qtadvanceddocking-qt5.lib"
)

# ---------------------------------------------------------------------
# OPM (opm-common) 静态库：ECLIPSE 输入文件解析
# 三段式布局：include + lib（静态库，无 DLL）。Debug/Release 自动切换。
# 链入 idos_providers 提供 .DATA 解析能力（Parser → Deck → EclipseState）。
# ---------------------------------------------------------------------
add_library(OPM::opmcommon STATIC IMPORTED)
set_target_properties(OPM::opmcommon PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/thirdparty/opm/include"
    IMPORTED_LOCATION_DEBUG   "${CMAKE_SOURCE_DIR}/thirdparty/opm/lib/opmcommond.lib"
    IMPORTED_LOCATION_RELEASE "${CMAKE_SOURCE_DIR}/thirdparty/opm/lib/opmcommon.lib"
    IMPORTED_LOCATION         "${CMAKE_SOURCE_DIR}/thirdparty/opm/lib/opmcommon.lib"
)

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