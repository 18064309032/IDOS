add_library(idossimulationplugin SHARED
    idossimulationplugin.cpp
    idossimulationplugin.h
)

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
    LIBRARY_OUTPUT_DIRECTORY "${_idos_simulation_plugin_output_directory}"
    RUNTIME_OUTPUT_DIRECTORY "${_idos_simulation_plugin_output_directory}"
)
