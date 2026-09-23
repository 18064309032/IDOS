# idos_render: VTK 渲染模块
# 渲染未实现期间（IDOS_ENABLE_RENDER=OFF）仍保留目标，供 app 结构性链接，
# 但不链接 VTK；下游可用 IDOS_ENABLE_RENDER 宏条件编译渲染调用
add_library(idos_render SHARED dummy.cpp)
target_include_directories(idos_render PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(idos_render PUBLIC idos_core)
target_compile_definitions(idos_render PRIVATE IDOS_RENDER_LIB)

if(IDOS_ENABLE_RENDER)
    # VTK 设 PRIVATE：头文件与库只进 idos_render 自己，
    # 下游（gui/app/exe）不继承 VTK include——防止 app 里
    # #include <vtkActor.h> 直接编过、绕过 render 边界。
    # 宏保持 PUBLIC：IDOS_ENABLE_RENDER 需要传播给下游做条件编译。
    # （exe 侧 VTK DLL 部署与直接链接见根/src CMakeLists 的 POST_BUILD。）
    target_link_libraries(idos_render PRIVATE ${VTK_LIBRARIES})
    target_compile_definitions(idos_render PUBLIC IDOS_ENABLE_RENDER)
endif()
