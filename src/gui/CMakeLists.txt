# idos_gui: user interface features
add_library(idos_gui SHARED
    idos_gui.h
    idosinterface.cpp
    idosinterface.h
    idosdebuginfowidget.cpp
    idosdebuginfowidget.h
    idosnewprojectdialog.cpp
    idosnewprojectdialog.h
    idosprojectstartwidget.cpp
    idosprojectstartwidget.h
    idospythonconsolewidget.cpp
    idospythonconsolewidget.h
    idosruntimeinfowidget.cpp
    idosruntimeinfowidget.h
    tree/idostreeview.cpp
    tree/idostreeview.h
    tree/idostreemenuprovider.cpp
    tree/idostreemenuprovider.h
)
target_include_directories(idos_gui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(idos_gui PUBLIC idos_core Qt5::Widgets)
target_compile_definitions(idos_gui PRIVATE IDOS_GUI_LIB)

get_target_property(_idos_sources idos_gui SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_idos_sources})

target_sources(idos_gui PRIVATE
    "${PROJECT_SOURCE_DIR}/images/images.qrc"
)
