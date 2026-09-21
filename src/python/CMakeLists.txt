# idos_python: Python C++ 运行类
add_library(idos_python SHARED dummy.cpp)
target_include_directories(idos_python PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(idos_python PUBLIC idos_core)
target_compile_definitions(idos_python PRIVATE IDOS_PYTHON_LIB)
