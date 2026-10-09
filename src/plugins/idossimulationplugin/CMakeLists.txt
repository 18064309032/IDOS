add_library(idossimulationplugin SHARED
    idossimulationplugin.qrc
    idossimulationplugin.cpp
    idossimulationplugin.h
)

idos_add_plugin_translation(idossimulationplugin)

target_include_directories(idossimulationplugin PRIVATE
    ${PROJECT_SOURCE_DIR}/src/app
)

target_link_libraries(idossimulationplugin PRIVATE
    idos_app
    Qt5::Core
)

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
