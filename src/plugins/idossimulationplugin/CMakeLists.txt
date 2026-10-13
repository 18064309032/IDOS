add_library(idossimulationplugin SHARED
    idossimulationplugin.qrc
    idossimulationplugin.cpp
    idossimulationplugin.h
    idoscoloringwidget.cpp
    idoscoloringwidget.h
    idoscoloringtreeguiprovider.cpp
    idoscoloringtreeguiprovider.h
    idosrendertoolmetadata.cpp
    idosrendertoolmetadata.h
    idosrendertoolprovider.cpp
    idosrendertoolprovider.h
    idosrenderinteractiontool.cpp
    idosrenderinteractiontool.h
    idossimulationcaseinventoryreportwidget.cpp
    idossimulationcaseinventoryreportwidget.h
    idossimulationcasetablewidget.cpp
    idossimulationcasetablewidget.h
    idossimulationcurvecomparisonwidget.cpp
    idossimulationcurvecomparisonwidget.h
    idossimulationdatagenerationwidget.cpp
    idossimulationdatagenerationwidget.h
    idossimulationdeckbrowserwidget.cpp
    idossimulationdeckbrowserwidget.h
    idossimulationinitializewidget.cpp
    idossimulationinitializewidget.h
    idossimulationparsesummarywidget.cpp
    idossimulationparsesummarywidget.h
    idossimulationpreflightwidget.cpp
    idossimulationpreflightwidget.h
    idossimulationpropertycurvewidget.cpp
    idossimulationpropertycurvewidget.h
    idossimulationresultfileswidget.cpp
    idossimulationresultfileswidget.h
    idossimulationresultplaybackwidget.cpp
    idossimulationresultplaybackwidget.h
    idossimulationrunarchivewidget.cpp
    idossimulationrunarchivewidget.h
    idossimulationrunstatuswidget.cpp
    idossimulationrunstatuswidget.h
    idossimulationwellcontrolswidget.cpp
    idossimulationwellcontrolswidget.h
    idossimulationworkflowwidget.cpp
    idossimulationworkflowwidget.h
)

idos_add_plugin_translation(idossimulationplugin)

target_include_directories(idossimulationplugin PRIVATE
    ${PROJECT_SOURCE_DIR}/src/app
)

target_link_libraries(idossimulationplugin PRIVATE
    idos_app
    Qt5::Core
)

if(IDOS_ENABLE_RENDER)
    target_link_libraries(idossimulationplugin PRIVATE
        VTK::CommonCore
        VTK::CommonDataModel
        VTK::CommonTransforms
        VTK::FiltersCore
        VTK::FiltersExtraction
        VTK::FiltersGeneral
        VTK::FiltersSources
        VTK::InteractionWidgets
        VTK::RenderingCore
        VTK::RenderingAnnotation
        VTK::GUISupportQt
    )
endif()

if(CMAKE_CONFIGURATION_TYPES)
    set(_idos_simulation_plugin_output_directory "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/plugins")
else()
    set(_idos_simulation_plugin_output_directory "${CMAKE_BINARY_DIR}/bin/plugins")
endif()

set_target_properties(idossimulationplugin PROPERTIES
    PREFIX ""
    DEBUG_POSTFIX ""
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/$<CONFIG>/plugins"
    LIBRARY_OUTPUT_DIRECTORY "${_idos_simulation_plugin_output_directory}"
    RUNTIME_OUTPUT_DIRECTORY "${_idos_simulation_plugin_output_directory}"
)
