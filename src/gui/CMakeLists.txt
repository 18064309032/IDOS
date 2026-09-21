# idos_gui: 带 UI 的部分
add_library(idos_gui SHARED
    dummy.cpp
    tree/idostreenode.cpp
    tree/idostreeview.cpp
    tree/idostreemenuprovider.cpp
    modeltree/idosmodeltreenode.cpp
    modeltree/idosmodeltreemodel.cpp
    modeltree/idosmodeltreemenuprovider.cpp
)
target_include_directories(idos_gui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/tree
        ${CMAKE_CURRENT_SOURCE_DIR}/modeltree
)
target_link_libraries(idos_gui PUBLIC idos_core Qt5::Widgets)
target_compile_definitions(idos_gui PRIVATE IDOS_GUI_LIB)
