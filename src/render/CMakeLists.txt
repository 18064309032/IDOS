# idos_render: VTK 渲染模块
add_library(idos_render SHARED dummy.cpp)
target_include_directories(idos_render PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(idos_render PUBLIC idos_core ${VTK_LIBRARIES})
target_compile_definitions(idos_render PRIVATE IDOS_RENDER_LIB)
