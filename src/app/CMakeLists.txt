# idos_app: 程序组装
add_library(idos_app SHARED dummy.cpp)
target_include_directories(idos_app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(idos_app PUBLIC
    idos_core
    idos_render
    idos_gui
    idos_python
    idos_assistant
    idos_process
    idos_providers
    Qt5::Core
    Qt5::Widgets
)
target_compile_definitions(idos_app PRIVATE IDOS_APP_LIB)
