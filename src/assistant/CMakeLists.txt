# idos_assistant: AI 助手
# 尚无实现，保留接口目标；有真实源文件后再建立共享库。
add_library(idos_assistant INTERFACE)
target_include_directories(idos_assistant INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(idos_assistant INTERFACE idos_core)
