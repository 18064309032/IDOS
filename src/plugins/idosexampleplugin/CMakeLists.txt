add_library(idosexampleplugin SHARED
    idosexampleplugin.h idosexampleplugin.cpp idosexampleplugin.def
    idospointset.h idospointset.cpp
    idospointsetmetadata.h idospointsetmetadata.cpp
    idospointsetcommands.h idospointsetcommands.cpp
    idospointsettreeprovider.h idospointsettreeprovider.cpp
    idospointsettreemetadata.h idospointsettreemetadata.cpp
    idospointsettreeguiprovider.h idospointsettreeguiprovider.cpp
    idospointsetrenderprovider.h idospointsetrenderprovider.cpp
    idospointsetrendermetadata.h idospointsetrendermetadata.cpp
    idospointsetrenderguiprovider.h idospointsetrenderguiprovider.cpp
)
target_include_directories(idosexampleplugin PRIVATE "${PROJECT_SOURCE_DIR}/src/plugins")
target_link_libraries(idosexampleplugin PRIVATE
    idos_core idos_gui idos_render
    Qt5::Widgets SARibbon::SARibbonBar
    VTK::CommonCore VTK::CommonDataModel VTK::RenderingCore
)
idos_add_plugin_translation(idosexampleplugin)
set_target_properties(idosexampleplugin PROPERTIES
    PREFIX ""
    DEBUG_POSTFIX ""
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/$<CONFIG>/plugins"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/plugins"
)

