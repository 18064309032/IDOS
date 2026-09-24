# idos_gui: 带 UI 的部分
add_library(idos_gui SHARED
    dummy.cpp
    idosmainwindow.cpp
    # 机制层：数据对象树的通用模型/节点/扩展点（不认识具体类型）
    tree/idostreenode.cpp
    tree/idostreeview.cpp
    tree/idostreemenuprovider.cpp
    tree/idostreepartkey.cpp
    tree/idostreegroupnode.cpp
    tree/idosobjecttreenode.cpp
    tree/idostreepartnode.cpp
    tree/idostreereferencenode.cpp
    tree/idostreebuilder.cpp
    tree/idostreeprovider.cpp
    tree/idosdatatreeprovider.cpp
    tree/idoscasetreeprovider.cpp
    tree/idostreeproviderregistry.cpp
    tree/idostreemodel.cpp
    # 数据树
    input/idosdatatreemodel.cpp
    input/idosdatatreeview.cpp
    input/providers/idoswelldatatreeprovider.cpp
    input/providers/idosgriddatatreeprovider.cpp
    # 新架构工况树
    case/idoscasetreemodel.cpp
    case/idoscasetreeview.cpp
    case/providers/idossimulationcasetreeprovider.cpp
)
target_include_directories(idos_gui
    PUBLIC
        # 仅暴露模块根：include 一律从根出发写全路径
        # （如 #include "tree/idostreemodel.h"），禁止 "../" 目录穿越
        ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(idos_gui PUBLIC idos_core Qt5::Widgets SARibbon::SARibbonBar ADS::ADS)
target_compile_definitions(idos_gui PRIVATE IDOS_GUI_LIB)
