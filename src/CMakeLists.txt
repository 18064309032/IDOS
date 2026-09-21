# src/CMakeLists.txt — 源码汇总层
# 按依赖顺序添加子模块：core(最底层) → render → gui → auto → app → idos(可执行)

add_subdirectory(core)
add_subdirectory(python)
add_subdirectory(render)
add_subdirectory(gui)
add_subdirectory(app)
add_subdirectory(assistant)
add_subdirectory(process)
add_subdirectory(providers)

# 可执行目标
add_executable(idos WIN32 main.cpp)

# CMAKE_DEBUG_POSTFIX 只初始化库目标的 <CONFIG>_POSTFIX（CMake 文档明确
# except executables），可执行文件需显式设置 → Debug 产物为 idosd.exe
set_target_properties(idos PROPERTIES DEBUG_POSTFIX "d")

target_link_libraries(idos PRIVATE
    idos_app
    Qt5::Core
    Qt5::Widgets
    Qt5::Network
)

if(IDOS_ENABLE_RENDER)
    target_link_libraries(idos PRIVATE ${VTK_LIBRARIES})
endif()

if(WIN32)
    # Deploy Qt DLLs (windeployqt analyzes Qt dependencies)
    # --compiler-runtime: Release 版部署 VC 运行时（Debug 版 windeployqt 不部署，见下）
    add_custom_command(TARGET idos POST_BUILD
        COMMAND "${IDOS_QT_DIR}/bin/windeployqt.exe" --compiler-runtime $<TARGET_FILE:idos>
        COMMENT "Deploying Qt DLLs via windeployqt..."
    )

    # windeployqt 不部署 Debug 版 VC 运行时（vcruntime140d.dll 等），
    # 从编译器路径反推 VS 安装根，定位 Redist 下的 DebugCRT 手动拷贝
    cmake_path(GET CMAKE_CXX_COMPILER PARENT_PATH _idos_p1)  # .../Hostx64/x64
    cmake_path(GET _idos_p1 PARENT_PATH _idos_p2)            # .../Hostx64
    cmake_path(GET _idos_p2 PARENT_PATH _idos_p3)            # .../bin
    cmake_path(GET _idos_p3 PARENT_PATH _idos_p4)            # .../MSVC/<ver>
    cmake_path(GET _idos_p4 PARENT_PATH _idos_p5)            # .../MSVC
    cmake_path(GET _idos_p5 PARENT_PATH _idos_p6)            # .../Tools
    cmake_path(GET _idos_p6 PARENT_PATH _idos_vcroot)        # .../VC
    file(GLOB _idos_debugcrt
        "${_idos_vcroot}/Redist/MSVC/*/debug_nonredist/x64/Microsoft.VC*.DebugCRT")
    list(SORT _idos_debugcrt ORDER DESCENDING)
    list(GET _idos_debugcrt 0 _idos_debugcrt_dir)
    if(EXISTS "${_idos_debugcrt_dir}")
        add_custom_command(TARGET idos POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${_idos_debugcrt_dir}" "$<TARGET_FILE_DIR:idos>"
            COMMENT "Copying VC Debug runtime DLLs..."
        )
    else()
        message(WARNING "VC Debug runtime redist not found under ${_idos_vcroot}/Redist")
    endif()

    # Deploy VTK DLLs (copy all; VTK has no equivalent of windeployqt)
    if(IDOS_ENABLE_RENDER)
        add_custom_command(TARGET idos POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/thirdparty/vtk9.6.0/bin"
                "$<TARGET_FILE_DIR:idos>"
            COMMENT "Copying VTK DLLs to exe directory..."
        )
    endif()
endif()
