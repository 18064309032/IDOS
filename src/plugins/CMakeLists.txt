function(idos_add_plugin_translation plugin_target)
    set(_idos_plugin_ts_file
        "${PROJECT_SOURCE_DIR}/i18n/${plugin_target}_zh_CN.ts")
    set(_idos_plugin_qm_file
        "${PROJECT_BINARY_DIR}/i18n/${plugin_target}_zh_CN.qm")
    set(_idos_plugin_translation_target "${plugin_target}_translations")

    if(CMAKE_CONFIGURATION_TYPES)
        set(_idos_plugin_translation_deploy_directory
            "${PROJECT_BINARY_DIR}/bin/$<CONFIG>/i18n")
    else()
        set(_idos_plugin_translation_deploy_directory "${PROJECT_BINARY_DIR}/bin/i18n")
    endif()

    add_custom_command(
        OUTPUT "${_idos_plugin_qm_file}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/i18n"
        COMMAND Qt5::lrelease "${_idos_plugin_ts_file}" -qm "${_idos_plugin_qm_file}"
        DEPENDS "${_idos_plugin_ts_file}"
        COMMENT "Compiling ${plugin_target} Chinese translations..."
        VERBATIM
    )
    add_custom_target("${_idos_plugin_translation_target}"
        DEPENDS "${_idos_plugin_qm_file}")
    add_dependencies("${plugin_target}" "${_idos_plugin_translation_target}")
    add_custom_target("${plugin_target}_lupdate"
        COMMAND Qt5::lupdate
            "${PROJECT_SOURCE_DIR}/src/plugins/${plugin_target}"
            -ts "${_idos_plugin_ts_file}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Extracting ${plugin_target} translatable strings..."
        VERBATIM
    )
    add_custom_command(TARGET "${plugin_target}" POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "${_idos_plugin_translation_deploy_directory}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${_idos_plugin_qm_file}"
            "${_idos_plugin_translation_deploy_directory}/${plugin_target}_zh_CN.qm"
        COMMENT "Deploying ${plugin_target} Chinese translations..."
        VERBATIM
    )
endfunction()

add_subdirectory(idossimulationplugin)
add_subdirectory(idosfracturemodelingplugin)
add_subdirectory(idosmultiscalefractureplugin)
add_subdirectory(idosgeomechanicsplugin)
