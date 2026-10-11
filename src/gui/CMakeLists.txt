# idos_gui: user interface features
add_library(idos_gui SHARED
    tree/idostreeview.h
    tree/idostreeview.cpp
    tree/idostreeguiprovider.h
    tree/idostreeguiprovider.cpp
    idos_gui.h
    idosinterface.h
    idosinterface.cpp
    idosdebuginfowidget.h
    idosdebuginfowidget.cpp
    idosruntimeinfowidget.h
    idosruntimeinfowidget.cpp
    idosnewprojectdialog.h
    idosnewprojectdialog.cpp
)
target_include_directories(idos_gui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/tree
)
target_link_libraries(idos_gui PUBLIC idos_core Qt5::Widgets)
target_compile_definitions(idos_gui PRIVATE IDOS_GUI_LIB)

get_target_property(_idos_sources idos_gui SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_idos_sources})

target_sources(idos_gui PRIVATE
    "${PROJECT_SOURCE_DIR}/images/images.qrc"
)
