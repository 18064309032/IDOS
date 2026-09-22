# idos_gui: 带 UI 的部分
add_library(idos_gui SHARED
    dummy.cpp
    idosmainwindow.cpp
    # 机制层：数据对象树的通用模型/节点/扩展点（不认识具体类型）
    tree/idostreenode.cpp
    tree/idostreeview.cpp
    tree/idostreemenuprovider.cpp
    tree/idosobjecttreenode.cpp
    tree/idosvaluetreenode.cpp
    tree/idostreemodel.cpp
    tree/idostreemodelmenuprovider.cpp
    # 输入树：TreeScope::Input + 输入数据的 provider/视图
    inputtree/idosinputtreemodel.cpp
    inputtree/idosinputtreeview.cpp
    inputtree/idoswellitemprovider.cpp
    # 模型树：TreeScope::Model + 模型对象的 provider/视图
    modeltree/idosmodeltreemodel.cpp
    modeltree/idosmodeltreeview.cpp
    modeltree/idosgriditemprovider.cpp
    modeltree/idosmodelitemprovider.cpp
)
target_include_directories(idos_gui
    PUBLIC
        # 仅暴露模块根：include 一律从根出发写全路径
        # （如 #include "tree/idostreemodel.h"），禁止 "../" 目录穿越
        ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(idos_gui PUBLIC idos_core Qt5::Widgets SARibbon::SARibbonBar ADS::ADS)
target_compile_definitions(idos_gui PRIVATE IDOS_GUI_LIB)
