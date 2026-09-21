# idos_process: 数据处理
add_library(idos_process SHARED dummy.cpp)
target_include_directories(idos_process PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(idos_process PUBLIC idos_core)
target_compile_definitions(idos_process PRIVATE IDOS_PROCESS_LIB)
