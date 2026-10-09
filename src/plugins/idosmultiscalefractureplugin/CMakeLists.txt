add_library(idosmultiscalefractureplugin SHARED
    idosmultiscalefractureplugin.qrc
    idosmultiscalefractureplugin.h
    idosmultiscalefractureplugin.cpp
)

idos_add_plugin_translation(idosmultiscalefractureplugin)

target_include_directories(idosmultiscalefractureplugin PRIVATE
    ${PROJECT_SOURCE_DIR}/src/app
)

target_link_libraries(idosmultiscalefractureplugin PRIVATE
    idos_app
    Qt5::Core
)

if(CMAKE_CONFIGURATION_TYPES)
    set(_idos_plugin_output_directory "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/plugins")
else()
    set(_idos_plugin_output_directory "${CMAKE_BINARY_DIR}/bin/plugins")
endif()

set_target_properties(idosmultiscalefractureplugin PROPERTIES
    PREFIX ""
    DEBUG_POSTFIX ""
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/$<CONFIG>/plugins"
    LIBRARY_OUTPUT_DIRECTORY "${_idos_plugin_output_directory}"
    RUNTIME_OUTPUT_DIRECTORY "${_idos_plugin_output_directory}"
)
