# idos_app: 程序组装
add_library(idos_app SHARED
    idos_app.h
    idosapplication.cpp
    idosapplication.h
    idoscasetreemenuprovider.h
    idoscasetreemenuprovider.cpp
    idosdatatreemenuprovider.h
    idosdatatreemenuprovider.cpp
    idosdataobjecthandling.h
    idosdataobjecthandling.cpp
    idosgridrenderobjectprovider.h
    idosgridrenderobjectprovider.cpp
    idoswellrenderobjectprovider.h
    idoswellrenderobjectprovider.cpp
    idoswelllogtrackview.h
    idoswelllogtrackview.cpp
    idospropertywidget.h
    idospropertywidget.cpp
    idosmainwindow.h
    idosmainwindow.cpp
    idosrenderobjectprovider.h
    idosrenderobjectprovider.cpp
    idosrenderserver.h
    idosrenderserver.cpp
)
target_include_directories(idos_app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(idos_app PUBLIC
    idos_core
    idos_gui
    idos_python
    idos_assistant
    idos_analysis
    idos_providers
    idos_render
    Qt5::Core
    SARibbon::SARibbonBar
    Qt5::Widgets
)
target_compile_definitions(idos_app PRIVATE IDOS_APP_LIB)

target_link_libraries(idos_app PRIVATE ADS::ADS)

get_target_property(_idos_sources idos_app SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_idos_sources})
