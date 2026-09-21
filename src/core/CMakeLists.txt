# idos_core: 业务数据，零自定义模块依赖
add_library(idos_core SHARED
    idosobject.cpp
    idosproject.cpp
    data/idosdataobject.cpp
    data/grid/idosgrid.cpp
    data/grid/idosgridcell.cpp
    data/well/idoswell.cpp
    data/well/idoswellcompletion.cpp
    data/well/idoswellsegment.cpp
    data/well/idoswellcontrol.cpp
    data/well/idoswellhead.cpp
)
target_include_directories(idos_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/data
        ${CMAKE_CURRENT_SOURCE_DIR}/data/grid
        ${CMAKE_CURRENT_SOURCE_DIR}/data/well
)
target_link_libraries(idos_core PUBLIC Qt5::Core Qt5::Gui)
target_compile_definitions(idos_core PRIVATE IDOS_CORE_LIB)
