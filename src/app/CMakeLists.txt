# idos_app: application composition
add_library(idos_app SHARED
    idos_app.h
    idosapplication.cpp
    idosapplication.h
    command/idosimportcommands.h
    command/idosimportcommands.cpp
    tree/idoscasetreemenuprovider.h
    tree/idoscasetreemenuprovider.cpp
    tree/idosdatatreemenuprovider.h
    tree/idosdatatreemenuprovider.cpp
    idosdataobjecthandling.h
    idosdataobjecthandling.cpp
    render/idosgridrenderobjectprovider.h
    render/idosgridrenderobjectprovider.cpp
    render/idoswellrenderobjectprovider.h
    render/idoswellrenderobjectprovider.cpp
    idoswelllogtrackview.h
    idoswelllogtrackview.cpp
    idospropertywidget.h
    idospropertywidget.cpp
    idosmainwindow.h
    idosmainwindow.cpp
    render/idosrenderobjectprovider.h
    render/idosrenderobjectprovider.cpp
    render/idosrenderserver.h
    render/idosrenderserver.cpp
)
target_include_directories(idos_app
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/command
        ${CMAKE_CURRENT_SOURCE_DIR}/render
        ${CMAKE_CURRENT_SOURCE_DIR}/tree
)
target_link_libraries(idos_app PUBLIC
    idos_core
    idos_gui
    idos_python
    idos_analysis
    idos_providers
    idos_render
    Qt5::Core
    SARibbon::SARibbonBar
    Qt5::Widgets
)
target_compile_definitions(idos_app PRIVATE IDOS_APP_LIB)

target_link_libraries(idos_app PRIVATE
    ADS::ADS
    idos_assistant
)
