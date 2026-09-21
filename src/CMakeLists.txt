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

target_link_libraries(idos PRIVATE
    idos_app
    Qt5::Core
    Qt5::Widgets
    Qt5::Network
    ${VTK_LIBRARIES}
)

if(WIN32)
    # Deploy Qt DLLs (windeployqt analyzes Qt dependencies)
    add_custom_command(TARGET idos POST_BUILD
        COMMAND "${IDOS_QT_DIR}/bin/windeployqt.exe" $<TARGET_FILE:idos>
        COMMENT "Deploying Qt DLLs via windeployqt..."
    )

    # Deploy VTK DLLs (copy all; VTK has no equivalent of windeployqt)
    add_custom_command(TARGET idos POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/thirdparty/vtk9.6.0/bin"
            "$<TARGET_FILE_DIR:idos>"
        COMMENT "Copying VTK DLLs to exe directory..."
    )
endif()
