add_library(idosfracturemodelingplugin SHARED
    idosfracturemodelingplugin.qrc
    idosfracturemodelingplugin.h
    idosfracturemodelingplugin.cpp
)

idos_add_plugin_translation(idosfracturemodelingplugin)

target_include_directories(idosfracturemodelingplugin PRIVATE
    ${PROJECT_SOURCE_DIR}/src/app
)

target_link_libraries(idosfracturemodelingplugin PRIVATE
    idos_app
    Qt5::Core
)

if(CMAKE_CONFIGURATION_TYPES)
    set(_idos_plugin_output_directory "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/plugins")
else()
    set(_idos_plugin_output_directory "${CMAKE_BINARY_DIR}/bin/plugins")
endif()

set_target_properties(idosfracturemodelingplugin PROPERTIES
    PREFIX ""
    DEBUG_POSTFIX ""
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/$<CONFIG>/plugins"
    LIBRARY_OUTPUT_DIRECTORY "${_idos_plugin_output_directory}"
    RUNTIME_OUTPUT_DIRECTORY "${_idos_plugin_output_directory}"
)
