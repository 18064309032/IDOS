# idos_providers: 数据提供者
add_library(idos_providers SHARED dummy.cpp)
target_include_directories(idos_providers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(idos_providers PUBLIC idos_core)
target_compile_definitions(idos_providers PRIVATE IDOS_PROVIDERS_LIB)
