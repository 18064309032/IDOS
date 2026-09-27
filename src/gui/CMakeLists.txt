# idos_gui: 带 UI 的部分
add_library(idos_gui SHARED
    idos_gui.h
    idoscasedialog.h
    idoscasedialog.cpp
    idoscasetreemodel.h
    idoscasetreemodel.cpp
    idoscasetreeprovider.h
    idoscasetreeprovider.cpp
    idoscasetreeview.h
    idoscasetreeview.cpp
    idosdatatreemodel.h
    idosdatatreemodel.cpp
    idosdatatreeprovider.h
    idosdatatreeprovider.cpp
    idosdatatreeview.h
    idosdatatreeview.cpp
    idosgriddatatreeprovider.h
    idosgriddatatreeprovider.cpp
    idosnewprojectdialog.h
    idosnewprojectdialog.cpp
    idosobjecttreenode.h
    idosobjecttreenode.cpp
    idossimulationcasetreeprovider.h
    idossimulationcasetreeprovider.cpp
    idostreebuilder.h
    idostreebuilder.cpp
    idostreegroupnode.h
    idostreegroupnode.cpp
    idostreemenuprovider.h
    idostreemenuprovider.cpp
    idostreemodel.h
    idostreemodel.cpp
    idostreenode.h
    idostreenode.cpp
    idostreepartkey.h
    idostreepartkey.cpp
    idostreepartnode.h
    idostreepartnode.cpp
    idostreeprovider.h
    idostreeprovider.cpp
    idostreeproviderregistry.h
    idostreeproviderregistry.cpp
    idostreereferencenode.h
    idostreereferencenode.cpp
    idostreeview.h
    idostreeview.cpp
    idoswelldatatreeprovider.h
    idoswelldatatreeprovider.cpp
    idoswellimportdialog.h
    idoswellimportdialog.cpp
    idoswellpathimportdialog.h
    idoswellpathimportdialog.cpp
)
target_include_directories(idos_gui
    PUBLIC
        # 公共类的头文件与实现文件成对放在模块根目录。
        ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(idos_gui PUBLIC idos_core Qt5::Widgets)
target_compile_definitions(idos_gui PRIVATE IDOS_GUI_LIB)

get_target_property(_idos_sources idos_gui SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_idos_sources})

target_sources(idos_gui PRIVATE
    "${PROJECT_SOURCE_DIR}/images/images.qrc"
)

