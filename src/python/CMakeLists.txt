# idos_python: Python C++ 运行类
# 尚无实现，保留接口目标；有真实源文件后再建立共享库。
add_library(idos_python INTERFACE)
target_include_directories(idos_python INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(idos_python INTERFACE idos_core)
