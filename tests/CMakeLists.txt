find_package(Qt5 COMPONENTS Test REQUIRED)
add_executable(idos_command_registry_tests command_registry_test.h command_registry_test.cpp)
target_link_libraries(idos_command_registry_tests PRIVATE idos_app Qt5::Test)
add_test(NAME command_registry COMMAND idos_command_registry_tests)
set_tests_properties(command_registry PROPERTIES
    TIMEOUT 30
    ENVIRONMENT "QT_QPA_PLATFORM=windows;QT_QPA_PLATFORM_PLUGIN_PATH=${IDOS_QT_DIR}/plugins/platforms;IDOS_TEST_SCREENSHOT_DIR=${CMAKE_CURRENT_BINARY_DIR}"
)

add_executable(idos_assistant_widget_tests assistant_widget_test.h assistant_widget_test.cpp)
target_link_libraries(idos_assistant_widget_tests PRIVATE idos_app Qt5::Test)
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
target_link_libraries(idos_case_tests PRIVATE idos_app Qt5::Test OPM::opmcommon)
add_dependencies(idos_case_tests idos_project_tests)
add_test(NAME case_workflow COMMAND idos_case_tests)
set_tests_properties(case_workflow PROPERTIES
    TIMEOUT 120
    ENVIRONMENT "QT_QPA_PLATFORM=windows;QT_QPA_PLATFORM_PLUGIN_PATH=${IDOS_QT_DIR}/plugins/platforms;IDOS_TEST_SCREENSHOT_DIR=${CMAKE_CURRENT_BINARY_DIR}"
)

add_executable(idos_tree_incremental_tests tree_incremental_test.h tree_incremental_test.cpp)
target_link_libraries(idos_tree_incremental_tests PRIVATE idos_app Qt5::Test)
add_dependencies(idos_tree_incremental_tests idos_project_tests)
add_test(NAME tree_incremental COMMAND idos_tree_incremental_tests)
set_tests_properties(tree_incremental PROPERTIES
    TIMEOUT 30
    ENVIRONMENT "QT_QPA_PLATFORM=windows;QT_QPA_PLATFORM_PLUGIN_PATH=${IDOS_QT_DIR}/plugins/platforms;IDOS_TEST_SCREENSHOT_DIR=${CMAKE_CURRENT_BINARY_DIR}"
)
