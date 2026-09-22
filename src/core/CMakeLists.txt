# idos_core: 业务数据 + 类型注册表
# 依赖链：零自定义模块依赖（只有 Qt5::Core Qt5::Gui）
add_library(idos_core SHARED
    idosobject.cpp
    idosproject.cpp
    idostyperegistry.cpp
    data/idosdataobject.cpp
    data/grid/idosgrid.cpp
    data/grid/idosgridcell.cpp
    data/grid/idosgridproperty.cpp
    data/model/idosmodel.cpp
    data/well/idoswell.cpp
    data/well/idoswellcompletion.cpp
    data/well/idoswellsegment.cpp
    data/well/idoswellcontrol.cpp
    data/well/idoswellhead.cpp
    data/well/idoswellpath.cpp
    data/well/idoswellpathpoint.cpp
    data/well/idoswelllogchannel.cpp
    data/well/idoswelllogset.cpp
    data/well/idoswellmarker.cpp
    data/well/idoswellmarkerset.cpp
)
target_include_directories(idos_core
    PUBLIC
        # 仅暴露模块根：include 一律从根出发写全路径
        # （如 #include "data/grid/idosgrid.h"）
        # 禁止 "../" 目录穿越
        ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(idos_core PUBLIC Qt5::Core Qt5::Gui)
target_compile_definitions(idos_core PRIVATE IDOS_CORE_LIB)
