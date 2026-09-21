# idos_assistant: AI 助手
add_library(idos_assistant SHARED dummy.cpp)
target_include_directories(idos_assistant PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(idos_assistant PUBLIC idos_core)
target_compile_definitions(idos_assistant PRIVATE IDOS_ASSISTANT_LIB)
