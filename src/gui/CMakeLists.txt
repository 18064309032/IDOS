# idos_gui: user interface features
add_library(idos_gui SHARED
    idos_gui.h
    idosassistantwidget.h
    idosassistantwidget.cpp
    idosdebuginfowidget.h
    idosdebuginfowidget.cpp
    idosruntimeinfowidget.h
    idosruntimeinfowidget.cpp
    idosnewprojectdialog.h
    idosnewprojectdialog.cpp
    case/idoscasedialog.h
    case/idoscasedialog.cpp
    tree/idoscasetreemodel.h
    tree/idoscasetreemodel.cpp
    tree/idoscasetreeprovider.h
    tree/idoscasetreeprovider.cpp
    tree/idoscasetreeview.h
    tree/idoscasetreeview.cpp
    tree/idosdatatreemodel.h
    tree/idosdatatreemodel.cpp
    tree/idosdatatreeprovider.h
    tree/idosdatatreeprovider.cpp
    tree/idosdatatreeview.h
    tree/idosdatatreeview.cpp
    tree/idosgriddatatreeprovider.h
    tree/idosgriddatatreeprovider.cpp
    tree/idosobjecttreenode.h
    tree/idosobjecttreenode.cpp
    tree/idossimulationcasetreeprovider.h
    tree/idossimulationcasetreeprovider.cpp
    tree/idostreebuilder.h
    tree/idostreebuilder.cpp
    tree/idostreegroupnode.h
    tree/idostreegroupnode.cpp
    tree/idostreemenuprovider.h
    tree/idostreemenuprovider.cpp
    tree/idostreemodel.h
    tree/idostreemodel.cpp
    tree/idostreenode.h
    tree/idostreenode.cpp
    tree/idostreepartkey.h
    tree/idostreepartkey.cpp
    tree/idostreepartnode.h
    tree/idostreepartnode.cpp
    tree/idostreeprovider.h
    tree/idostreeprovider.cpp
    tree/idostreeproviderregistry.h
    tree/idostreeproviderregistry.cpp
    tree/idostreereferencenode.h
    tree/idostreereferencenode.cpp
    tree/idostreeview.h
    tree/idostreeview.cpp
    tree/idoswelldatatreeprovider.h
    tree/idoswelldatatreeprovider.cpp
)
target_include_directories(idos_gui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/case
        ${CMAKE_CURRENT_SOURCE_DIR}/tree
)
target_link_libraries(idos_gui PUBLIC idos_core idos_assistant Qt5::Widgets)
target_compile_definitions(idos_gui PRIVATE IDOS_GUI_LIB)

get_target_property(_idos_sources idos_gui SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_idos_sources})

target_sources(idos_gui PRIVATE
    "${PROJECT_SOURCE_DIR}/images/images.qrc"
)
