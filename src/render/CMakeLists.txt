# idos_render: VTK 渲染模块
# 渲染未实现期间（IDOS_ENABLE_RENDER=OFF）仍保留目标，供 app 结构性链接，
# 但不链接 VTK；下游可用 IDOS_ENABLE_RENDER 宏条件编译渲染调用
add_library(idos_render SHARED dummy.cpp)
target_include_directories(idos_render PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(idos_render PUBLIC idos_core)
target_compile_definitions(idos_render PRIVATE IDOS_RENDER_LIB)

if(IDOS_ENABLE_RENDER)
    target_link_libraries(idos_render PUBLIC ${VTK_LIBRARIES})
    target_compile_definitions(idos_render PUBLIC IDOS_ENABLE_RENDER)
endif()
