find_package(Qt5 COMPONENTS Test REQUIRED)

# Compile each rendering header independently to enforce self-contained APIs.
set(_idos_render_headers
    idosinterface.h idosappinterface.h idosassistantwidget.h
    idosgridrendermetadata.h idosgridrenderprovider.h
    idosimportrestartresultscommand.h
    idoswellrendermetadata.h idoswellrenderprovider.h

    idosrendermetadata.h idosrenderregistry.h idosrenderserver.h
    idostool.h idostoolmetadata.h idostoolregistry.h idostoolprovider.h idostoolbar.h
    idosrenderobject.h idosrenderprovider.h idosrenderscene.h idosrenderview.h
    idostree.h idostreeitem.h idostreeprovider.h idostreemetadata.h
    idostreeregistry.h idostreemodel.h idostreeserver.h
    idostreeview.h idostreeguiprovider.h
    idoswelltreeprovider.h idoswelltreemetadata.h idoswelltreeguiprovider.h
    idosgridtreeprovider.h idosgridtreemetadata.h idosgridtreeguiprovider.h
    idossimulationcasetreeprovider.h idossimulationcasetreemetadata.h idossimulationcasetreeguiprovider.h
)
set(_idos_render_header_sources)
file(GLOB _idos_example_headers CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/plugins/idosexampleplugin/*.h")
list(APPEND _idos_render_headers ${_idos_example_headers})
list(APPEND _idos_render_headers
    "${CMAKE_SOURCE_DIR}/src/plugins/idossimulationplugin/idossimulationplugin.h"
    "${CMAKE_SOURCE_DIR}/src/plugins/idossimulationplugin/idossimulationresultplaybackwidget.h"
    "${CMAKE_SOURCE_DIR}/src/plugins/idossimulationplugin/idossimulationinitializewidget.h"
    "${CMAKE_SOURCE_DIR}/src/plugins/idossimulationplugin/idossimulationdatagenerationwidget.h"
    "${CMAKE_SOURCE_DIR}/src/plugins/idossimulationplugin/idossimulationrunarchivewidget.h"
    "${CMAKE_SOURCE_DIR}/src/plugins/idossimulationplugin/idossimulationrunstatuswidget.h")
foreach(_idos_header IN LISTS _idos_render_headers)
    get_filename_component(_idos_header_name "${_idos_header}" NAME)
    set(_idos_header_source "${CMAKE_CURRENT_BINARY_DIR}/header_${_idos_header_name}.cpp")
    file(WRITE "${_idos_header_source}" "#include \"${_idos_header}\"\n")
    list(APPEND _idos_render_header_sources "${_idos_header_source}")
endforeach()
add_library(idos_render_header_checks OBJECT ${_idos_render_header_sources})
target_link_libraries(idos_render_header_checks PRIVATE idos_app idos_assistant)
target_include_directories(idos_render_header_checks PRIVATE "${CMAKE_SOURCE_DIR}/src/plugins")
target_include_directories(idos_render_header_checks PRIVATE "${CMAKE_SOURCE_DIR}/src/app/well" "${CMAKE_SOURCE_DIR}/src/app/grid" "${CMAKE_SOURCE_DIR}/src/app/case")
add_executable(idos_command_registry_tests command_registry_test.h command_registry_test.cpp)
target_link_libraries(idos_command_registry_tests PRIVATE idos_app Qt5::Test)
add_test(NAME command_registry COMMAND idos_command_registry_tests)
set_tests_properties(command_registry PROPERTIES
    TIMEOUT 30
    ENVIRONMENT "QT_QPA_PLATFORM=windows;QT_QPA_PLATFORM_PLUGIN_PATH=${IDOS_QT_DIR}/plugins/platforms;IDOS_TEST_SCREENSHOT_DIR=${CMAKE_CURRENT_BINARY_DIR}"
)

add_executable(idos_assistant_widget_tests assistant_widget_test.h assistant_widget_test.cpp)
target_link_libraries(idos_assistant_widget_tests PRIVATE idos_app idos_assistant Qt5::Test)
add_test(NAME assistant_widget COMMAND idos_assistant_widget_tests)
set_tests_properties(assistant_widget PROPERTIES
    TIMEOUT 30
    ENVIRONMENT "QT_QPA_PLATFORM=windows;QT_QPA_PLATFORM_PLUGIN_PATH=${IDOS_QT_DIR}/plugins/platforms;IDOS_TEST_SCREENSHOT_DIR=${CMAKE_CURRENT_BINARY_DIR}"
)

add_executable(idos_project_tests project_dialog_test.cpp)
target_link_libraries(idos_project_tests PRIVATE idos_app Qt5::Test)
add_dependencies(idos_project_tests idos translations)
add_custom_command(TARGET idos_project_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Qt5::Test> $<TARGET_FILE_DIR:idos_project_tests>
)
add_test(NAME project_dialog COMMAND idos_project_tests)
set_tests_properties(project_dialog PROPERTIES
    TIMEOUT 30
    ENVIRONMENT "QT_QPA_PLATFORM=windows;QT_QPA_PLATFORM_PLUGIN_PATH=${IDOS_QT_DIR}/plugins/platforms;IDOS_TEST_SCREENSHOT_DIR=${CMAKE_CURRENT_BINARY_DIR}"
)

add_executable(idos_well_import_tests well_import_test.h well_import_test.cpp)
target_link_libraries(idos_well_import_tests PRIVATE idos_app Qt5::Test)
add_test(NAME well_import COMMAND idos_well_import_tests)
set_tests_properties(well_import PROPERTIES
    TIMEOUT 30
    ENVIRONMENT "QT_QPA_PLATFORM=windows;QT_QPA_PLATFORM_PLUGIN_PATH=${IDOS_QT_DIR}/plugins/platforms;IDOS_TEST_SCREENSHOT_DIR=${CMAKE_CURRENT_BINARY_DIR}"
)

add_executable(idos_well_path_tests well_path_test.h well_path_test.cpp)
target_link_libraries(idos_well_path_tests PRIVATE idos_app Qt5::Test)
add_dependencies(idos_well_path_tests idos_project_tests)
add_test(NAME well_path COMMAND idos_well_path_tests)
set_tests_properties(well_path PROPERTIES
    TIMEOUT 30
    ENVIRONMENT "QT_QPA_PLATFORM=windows;QT_QPA_PLATFORM_PLUGIN_PATH=${IDOS_QT_DIR}/plugins/platforms;IDOS_TEST_SCREENSHOT_DIR=${CMAKE_CURRENT_BINARY_DIR}"
)

add_executable(idos_case_tests case_workflow_test.h case_workflow_test.cpp)
target_sources(idos_case_tests PRIVATE
    "${CMAKE_SOURCE_DIR}/src/plugins/idossimulationplugin/idossimulationresultplaybackwidget.cpp"
    "${CMAKE_SOURCE_DIR}/src/plugins/idossimulationplugin/idossimulationresultplaybackwidget.h"
    "${CMAKE_SOURCE_DIR}/src/plugins/idossimulationplugin/idossimulationinitializewidget.cpp"
    "${CMAKE_SOURCE_DIR}/src/plugins/idossimulationplugin/idossimulationinitializewidget.h"
    "${CMAKE_SOURCE_DIR}/src/plugins/idossimulationplugin/idossimulationrunstatuswidget.cpp"
    "${CMAKE_SOURCE_DIR}/src/plugins/idossimulationplugin/idossimulationrunstatuswidget.h")
target_include_directories(idos_case_tests PRIVATE "${CMAKE_SOURCE_DIR}/src/plugins/idossimulationplugin")
target_link_libraries(idos_case_tests PRIVATE idos_app Qt5::Test OPM::opmcommon)
target_include_directories(idos_case_tests PRIVATE "${CMAKE_SOURCE_DIR}/src/app/well" "${CMAKE_SOURCE_DIR}/src/app/grid")
target_link_libraries(idos_case_tests PRIVATE VTK::CommonCore VTK::CommonDataModel VTK::RenderingCore)
add_dependencies(idos_case_tests idos_project_tests)
add_dependencies(idos_case_tests idosexampleplugin)
add_test(NAME case_workflow COMMAND idos_case_tests)
set_tests_properties(case_workflow PROPERTIES
    TIMEOUT 240
    ENVIRONMENT "QT_QPA_PLATFORM=windows;QT_QPA_PLATFORM_PLUGIN_PATH=${IDOS_QT_DIR}/plugins/platforms;IDOS_TEST_SCREENSHOT_DIR=${CMAKE_CURRENT_BINARY_DIR}"
)

add_executable(idos_tree_incremental_tests tree_incremental_test.h tree_incremental_test.cpp)
target_link_libraries(idos_tree_incremental_tests PRIVATE idos_app Qt5::Test VTK::CommonCore VTK::GUISupportQt)
add_dependencies(idos_tree_incremental_tests idos_project_tests)
add_test(NAME tree_incremental COMMAND idos_tree_incremental_tests)
set_tests_properties(tree_incremental PROPERTIES
    TIMEOUT 30
    ENVIRONMENT "QT_QPA_PLATFORM=windows;QT_QPA_PLATFORM_PLUGIN_PATH=${IDOS_QT_DIR}/plugins/platforms;IDOS_TEST_SCREENSHOT_DIR=${CMAKE_CURRENT_BINARY_DIR}"
)

# Tests exercise the built-in implementations without exposing them as public API.
foreach(_idos_test_target IN ITEMS
    idos_command_registry_tests idos_assistant_widget_tests idos_project_tests
    idos_well_import_tests idos_well_path_tests idos_case_tests
    idos_tree_incremental_tests)
    target_include_directories(${_idos_test_target} PRIVATE
        "${CMAKE_SOURCE_DIR}/src/app/well"
        "${CMAKE_SOURCE_DIR}/src/app/grid"
        "${CMAKE_SOURCE_DIR}/src/app/case")
endforeach()
