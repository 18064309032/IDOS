find_package(Qt5 COMPONENTS Test REQUIRED)

# Compile platform and example extension headers independently.
set(_idos_render_headers
    "${CMAKE_SOURCE_DIR}/src/core/idosprojectmetadata.h"
    "${CMAKE_SOURCE_DIR}/src/gui/idosnewprojectdialog.h"
    "${CMAKE_SOURCE_DIR}/src/app/command/idosconvertcoordinatescommand.h"
    "${CMAKE_SOURCE_DIR}/src/app/command/idosupdateprojectmetadatacommand.h"
    "${CMAKE_SOURCE_DIR}/src/plugins/idosmultiscalefractureplugin/idosfielddifferencecommand.h"
    "${CMAKE_SOURCE_DIR}/src/plugins/idosmultiscalefractureplugin/idosfielddifferencewidget.h"
    "${CMAKE_SOURCE_DIR}/src/plugins/idosmultiscalefractureplugin/idoscurveevaluation.h"
    "${CMAKE_SOURCE_DIR}/src/plugins/idosmultiscalefractureplugin/idoscurveevaluationcommand.h"
    "${CMAKE_SOURCE_DIR}/src/plugins/idosmultiscalefractureplugin/idoscurvefitevaluationwidget.h"
    "${CMAKE_SOURCE_DIR}/src/app/data/idosnaturalfracturerendermetadata.h"
    "${CMAKE_SOURCE_DIR}/src/app/data/idosnaturalfracturerenderprovider.h"
    "${CMAKE_SOURCE_DIR}/src/app/command/idoscasereferencecommand.h"
    "${CMAKE_SOURCE_DIR}/src/app/command/idoscreatecasecommand.h"
    "${CMAKE_SOURCE_DIR}/src/app/command/idoscreatewellcommand.h"
    "${CMAKE_SOURCE_DIR}/src/app/command/idosdeletecasecommand.h"
    "${CMAKE_SOURCE_DIR}/src/app/command/idosdeletegridcommand.h"
    "${CMAKE_SOURCE_DIR}/src/app/command/idosdeletepropertycommand.h"
    "${CMAKE_SOURCE_DIR}/src/app/command/idosdeletewellcommand.h"
    "${CMAKE_SOURCE_DIR}/src/app/command/idoscreategridfiltercommand.h"
    "${CMAKE_SOURCE_DIR}/src/app/command/idosupdategridfiltercommand.h"
    "${CMAKE_SOURCE_DIR}/src/app/command/idosdeletegridfiltercommand.h"
    "${CMAKE_SOURCE_DIR}/src/app/command/idosimportcasecommand.h"
    "${CMAKE_SOURCE_DIR}/src/app/command/idosimportgridcommand.h"
    "${CMAKE_SOURCE_DIR}/src/app/command/idosimportgridpropertycommand.h"
    "${CMAKE_SOURCE_DIR}/src/app/command/idosimportwelldatacommand.h"
    "${CMAKE_SOURCE_DIR}/src/app/command/idosimportfluidrocktablescommand.h"
    "${CMAKE_SOURCE_DIR}/src/app/command/idosimportdatacommand.h"
    "${CMAKE_SOURCE_DIR}/src/app/command/idosimportsummaryresultscommand.h"
    "${CMAKE_SOURCE_DIR}/src/app/command/idosimportrestartresultscommand.h"
    "${CMAKE_SOURCE_DIR}/src/app/command/idosrenameobjectcommand.h"
    "${CMAKE_SOURCE_DIR}/src/app/render/idospointzoomtoolprovider.h"
    "${CMAKE_SOURCE_DIR}/src/app/render/idospointprobetoolprovider.h"
    "${CMAKE_SOURCE_DIR}/src/app/render/idosdistancemeasurementtoolprovider.h"
    "${CMAKE_SOURCE_DIR}/src/app/render/idosselectiontoolprovider.h"
    "${CMAKE_SOURCE_DIR}/src/app/render/idosclippingplanetoolprovider.h"
    "${CMAKE_SOURCE_DIR}/src/app/render/idosslicebypointstoolprovider.h"
    "${CMAKE_SOURCE_DIR}/src/app/idos_app.h"
    "${CMAKE_SOURCE_DIR}/src/app/idosappinterface.h"
    "${CMAKE_SOURCE_DIR}/src/app/idosapplication.h"
    "${CMAKE_SOURCE_DIR}/src/app/idosmainwindow.h"
    "${CMAKE_SOURCE_DIR}/src/app/case/idoscasetreerootprovider.h"
    "${CMAKE_SOURCE_DIR}/src/app/data/idosdatatreerootprovider.h"
    "${CMAKE_SOURCE_DIR}/src/app/case/idoscasetreemenuprovider.h"
    "${CMAKE_SOURCE_DIR}/src/app/data/idosdatatreemenuprovider.h"
    "${CMAKE_SOURCE_DIR}/src/gui/tree/idostreemenuprovider.h"
    "${CMAKE_SOURCE_DIR}/src/app/plugin/idospluginmetadata.h"
    "${CMAKE_SOURCE_DIR}/src/app/plugin/idospluginregistry.h"
    "${CMAKE_SOURCE_DIR}/src/app/simulation/idossimulation.h"
    "${CMAKE_SOURCE_DIR}/src/app/plugin/idospluginlistmodel.h"
    "${CMAKE_SOURCE_DIR}/src/app/plugin/idospluginmanagerwidget.h"
    "${CMAKE_SOURCE_DIR}/src/app/fluidrock/idosfluidrocktabletreemetadata.h"
    "${CMAKE_SOURCE_DIR}/src/app/fluidrock/idosfluidrocktabletreeprovider.h"
    "${CMAKE_SOURCE_DIR}/src/app/grid/idosgridboundaryrendermetadata.h"
    "${CMAKE_SOURCE_DIR}/src/app/grid/idosgridboundaryrenderprovider.h"
    "${CMAKE_SOURCE_DIR}/src/app/grid/idosgridfilterrendermetadata.h"
    "${CMAKE_SOURCE_DIR}/src/app/grid/idosgridfilterrenderprovider.h"
    "${CMAKE_SOURCE_DIR}/src/app/grid/idosgridrendermetadata.h"
    "${CMAKE_SOURCE_DIR}/src/app/grid/idosgridrenderprovider.h"
    "${CMAKE_SOURCE_DIR}/src/app/grid/idosgridsurfacerendermetadata.h"
    "${CMAKE_SOURCE_DIR}/src/app/grid/idosgridsurfacerenderprovider.h"
    "${CMAKE_SOURCE_DIR}/src/app/grid/idosgridtreemetadata.h"
    "${CMAKE_SOURCE_DIR}/src/app/grid/idosgridtreeprovider.h"
    "${CMAKE_SOURCE_DIR}/src/app/data/idosimporteddatatreemetadata.h"
    "${CMAKE_SOURCE_DIR}/src/app/data/idosimporteddatatreeprovider.h"
    "${CMAKE_SOURCE_DIR}/src/app/seismic/idosseismicvolumerendermetadata.h"
    "${CMAKE_SOURCE_DIR}/src/app/seismic/idosseismicvolumerenderprovider.h"
    "${CMAKE_SOURCE_DIR}/src/app/case/idossimulationcasetreemetadata.h"
    "${CMAKE_SOURCE_DIR}/src/app/case/idossimulationcasetreeprovider.h"
    "${CMAKE_SOURCE_DIR}/src/app/well/idoswellrendermetadata.h"
    "${CMAKE_SOURCE_DIR}/src/app/well/idoswellrenderprovider.h"
    "${CMAKE_SOURCE_DIR}/src/app/well/idoswelltreemetadata.h"
    "${CMAKE_SOURCE_DIR}/src/app/well/idoswelltreeprovider.h"
    "${CMAKE_SOURCE_DIR}/src/app/render/idosrendertoolmetadata.h"
    "${CMAKE_SOURCE_DIR}/src/app/render/idosrendertoolprovider.h"
    "${CMAKE_SOURCE_DIR}/src/assistant/idos_assistant.h"
    "${CMAKE_SOURCE_DIR}/src/assistant/idosassistantservice.h"
    "${CMAKE_SOURCE_DIR}/src/assistant/idosassistantwidget.h"
    "${CMAKE_SOURCE_DIR}/src/assistant/idoslocalmodelprocess.h"
    "${CMAKE_SOURCE_DIR}/src/assistant/idoslocalmodelservice.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/case/idoscaseitemref.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/case/idoscaseobject.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/case/idossimulationcasemetadata.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/case/idossimulationcaseobject.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/fluidrock/idosfluidrocktableobject.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/fluidrock/idosfluidrocktablemetadata.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/fracture/idosfracturematerialcatalogobject.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/fracture/idosfracturematerialcatalogdatametadata.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/fracture/idosfracturescheduleobject.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/fracture/idosfracturescheduledatametadata.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/fracture/idosnaturalfracturesetobject.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/fracture/idosnaturalfracturesetdatametadata.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/grid/idosgridobject.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/grid/idosgridboundaryobject.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/grid/idosgridboundarydatametadata.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/grid/idosgridcell.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/grid/idosgridfilterobject.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/grid/idosgridfiltermetadata.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/grid/idosgridmetadata.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/grid/idosgridpropertyobject.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/grid/idosgridpropertymetadata.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/grid/idosgridsurfaceobject.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/grid/idosgridsurfacedatametadata.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/production/idosproductionhistoryobject.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/production/idosproductionhistorydatametadata.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/seismic/idosmicroseismicdatametadata.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/seismic/idosmicroseismicsetobject.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/seismic/idosseismicvolumeobject.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/seismic/idosseismicvolumedatametadata.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/well/idoswellobject.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/well/idoswellcheckshot.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/well/idoswellcompletion.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/well/idoswellcontrol.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/well/idoswellgeometryresolver.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/well/idoswellhead.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/well/idoswelllogchannel.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/well/idoswelllogset.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/well/idoswellmarker.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/well/idoswellmarkerset.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/well/idoswellmetadata.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/well/idoswellpath.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/well/idoswellpathpoint.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/well/idoswellsegment.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/well/idoswellspatialreference.h"
    "${CMAKE_SOURCE_DIR}/src/core/idos_core.h"
    "${CMAKE_SOURCE_DIR}/src/core/command/idoscommand.h"
    "${CMAKE_SOURCE_DIR}/src/core/command/idoscommandmanager.h"
    "${CMAKE_SOURCE_DIR}/src/core/command/idoscommandmetadata.h"
    "${CMAKE_SOURCE_DIR}/src/core/command/idoscommandregistry.h"
    "${CMAKE_SOURCE_DIR}/src/core/idosdataobject.h"
    "${CMAKE_SOURCE_DIR}/src/core/idosobject.h"
    "${CMAKE_SOURCE_DIR}/src/core/idosproject.h"
    "${CMAKE_SOURCE_DIR}/src/core/idosprojectmetadata.h"
    "${CMAKE_SOURCE_DIR}/src/core/logger/idosconsoletarget.h"
    "${CMAKE_SOURCE_DIR}/src/core/logger/idosfiletarget.h"
    "${CMAKE_SOURCE_DIR}/src/core/logger/idoslogger.h"
    "${CMAKE_SOURCE_DIR}/src/core/logger/idosloglevel.h"
    "${CMAKE_SOURCE_DIR}/src/core/logger/idoslogrecord.h"
    "${CMAKE_SOURCE_DIR}/src/core/logger/idoslogtarget.h"
    "${CMAKE_SOURCE_DIR}/src/core/logger/idosmsvctarget.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/idosdatametadata.h"
    "${CMAKE_SOURCE_DIR}/src/core/providers/idosdataprovider.h"
    "${CMAKE_SOURCE_DIR}/src/core/providers/idosdataprovidermetadata.h"
    "${CMAKE_SOURCE_DIR}/src/core/providers/idosdataproviderregistry.h"
    "${CMAKE_SOURCE_DIR}/src/core/data/idosdataregistry.h"
    "${CMAKE_SOURCE_DIR}/src/core/tree/idostreeitemprovidermetadata.h"
    "${CMAKE_SOURCE_DIR}/src/core/tree/idostreeitemprovider.h"
    "${CMAKE_SOURCE_DIR}/src/core/tree/idostreeitemproviderregistry.h"
    "${CMAKE_SOURCE_DIR}/src/core/tree/idostreeaccess.h"
    "${CMAKE_SOURCE_DIR}/src/app/idosapptreeaccess.h"
    "${CMAKE_SOURCE_DIR}/src/core/tree/idostreemodel.h"
    "${CMAKE_SOURCE_DIR}/src/core/tree/idostreeitem.h"
    "${CMAKE_SOURCE_DIR}/src/gui/idos_gui.h"
    "${CMAKE_SOURCE_DIR}/src/app/case/idoscasedialog.h"
    "${CMAKE_SOURCE_DIR}/src/gui/idosdebuginfowidget.h"
    "${CMAKE_SOURCE_DIR}/src/app/grid/idosgridfilterwidget.h"
    "${CMAKE_SOURCE_DIR}/src/app/grid/idosgridsettingwidget.h"
    "${CMAKE_SOURCE_DIR}/src/gui/idosinterface.h"
    "${CMAKE_SOURCE_DIR}/src/gui/idosnewprojectdialog.h"
    "${CMAKE_SOURCE_DIR}/src/gui/idospythonconsolewidget.h"
    "${CMAKE_SOURCE_DIR}/src/gui/idosruntimeinfowidget.h"
    "${CMAKE_SOURCE_DIR}/src/app/case/idossimulationcaseinventoryreportwidget.h"
    "${CMAKE_SOURCE_DIR}/src/app/case/idossimulationcasetablewidget.h"
    "${CMAKE_SOURCE_DIR}/src/app/case/idossimulationcurvecomparisonwidget.h"
    "${CMAKE_SOURCE_DIR}/src/app/case/idossimulationdatagenerationwidget.h"
    "${CMAKE_SOURCE_DIR}/src/app/case/idossimulationinitializewidget.h"
    "${CMAKE_SOURCE_DIR}/src/app/case/idossimulationpreflightwidget.h"
    "${CMAKE_SOURCE_DIR}/src/app/case/idossimulationpropertycurvewidget.h"
    "${CMAKE_SOURCE_DIR}/src/app/case/idossimulationresultfileswidget.h"
    "${CMAKE_SOURCE_DIR}/src/app/case/idossimulationresultplaybackwidget.h"
    "${CMAKE_SOURCE_DIR}/src/app/case/idossimulationrunarchivewidget.h"
    "${CMAKE_SOURCE_DIR}/src/app/case/idossimulationrunstatuswidget.h"
    "${CMAKE_SOURCE_DIR}/src/app/case/idossimulationwellcontrolswidget.h"
    "${CMAKE_SOURCE_DIR}/src/gui/tree/idostreeview.h"
    "${CMAKE_SOURCE_DIR}/src/app/well/idoswelldisplaywidget.h"
    "${CMAKE_SOURCE_DIR}/src/app/well/idoswelllogtrackview.h"
    "${CMAKE_SOURCE_DIR}/src/plugins/idosexampleplugin/idosexampleplugin.h"
    "${CMAKE_SOURCE_DIR}/src/plugins/idosexampleplugin/idospointsetobject.h"
    "${CMAKE_SOURCE_DIR}/src/plugins/idosexampleplugin/idospointsetcommands.h"
    "${CMAKE_SOURCE_DIR}/src/plugins/idosexampleplugin/idospointsetmetadata.h"
    "${CMAKE_SOURCE_DIR}/src/plugins/idosexampleplugin/idospointsetrendermetadata.h"
    "${CMAKE_SOURCE_DIR}/src/plugins/idosexampleplugin/idospointsetrenderprovider.h"
    "${CMAKE_SOURCE_DIR}/src/plugins/idosexampleplugin/idospointsettreemetadata.h"
    "${CMAKE_SOURCE_DIR}/src/plugins/idosexampleplugin/idospointsettreeprovider.h"
    "${CMAKE_SOURCE_DIR}/src/providers/case/idossimulationcaseeclipsemetadata.h"
    "${CMAKE_SOURCE_DIR}/src/providers/case/idossimulationcaseeclipseprovider.h"
    "${CMAKE_SOURCE_DIR}/src/providers/fluidrock/idosfluidrocktextprovider.h"
    "${CMAKE_SOURCE_DIR}/src/providers/fracture/idosfracturematerialcatalogmetadata.h"
    "${CMAKE_SOURCE_DIR}/src/providers/fracture/idosfracturematerialcatalogprovider.h"
    "${CMAKE_SOURCE_DIR}/src/providers/fracture/idosfractureschedulemetadata.h"
    "${CMAKE_SOURCE_DIR}/src/providers/fracture/idosfracturescheduleprovider.h"
    "${CMAKE_SOURCE_DIR}/src/providers/fracture/idosnaturalfracturemetadata.h"
    "${CMAKE_SOURCE_DIR}/src/providers/fracture/idosnaturalfractureprovider.h"
    "${CMAKE_SOURCE_DIR}/src/providers/grid/idosgridboundarymetadata.h"
    "${CMAKE_SOURCE_DIR}/src/providers/grid/idosgridboundaryprovider.h"
    "${CMAKE_SOURCE_DIR}/src/providers/grid/idosgrideclipsemetadata.h"
    "${CMAKE_SOURCE_DIR}/src/providers/grid/idosgrideclipseprovider.h"
    "${CMAKE_SOURCE_DIR}/src/providers/grid/idosgridpropertyeclipsemetadata.h"
    "${CMAKE_SOURCE_DIR}/src/providers/grid/idosgridpropertyeclipseprovider.h"
    "${CMAKE_SOURCE_DIR}/src/providers/grid/idosgridsurfacemetadata.h"
    "${CMAKE_SOURCE_DIR}/src/providers/grid/idosgridsurfaceprovider.h"
    "${CMAKE_SOURCE_DIR}/src/providers/idos_providers.h"
    "${CMAKE_SOURCE_DIR}/src/providers/las/idoswelllasmetadata.h"
    "${CMAKE_SOURCE_DIR}/src/providers/las/idoswelllasprovider.h"
    "${CMAKE_SOURCE_DIR}/src/providers/production/idosproductionhistorymetadata.h"
    "${CMAKE_SOURCE_DIR}/src/providers/production/idosproductionhistoryprovider.h"
    "${CMAKE_SOURCE_DIR}/src/providers/seismic/idosmicroseismicmetadata.h"
    "${CMAKE_SOURCE_DIR}/src/providers/seismic/idosmicroseismicprovider.h"
    "${CMAKE_SOURCE_DIR}/src/providers/seismic/idossegyprovider.h"
    "${CMAKE_SOURCE_DIR}/src/providers/seismic/idossegyprovidermetadata.h"
    "${CMAKE_SOURCE_DIR}/src/providers/well/idoswellcheckshotmetadata.h"
    "${CMAKE_SOURCE_DIR}/src/providers/well/idoswellcheckshotprovider.h"
    "${CMAKE_SOURCE_DIR}/src/providers/well/idoswellcompletionmetadata.h"
    "${CMAKE_SOURCE_DIR}/src/providers/well/idoswellcompletionprovider.h"
    "${CMAKE_SOURCE_DIR}/src/providers/well/idoswellheadermetadata.h"
    "${CMAKE_SOURCE_DIR}/src/providers/well/idoswellheaderprovider.h"
    "${CMAKE_SOURCE_DIR}/src/providers/well/idoswellpathmetadata.h"
    "${CMAKE_SOURCE_DIR}/src/providers/well/idoswellpathprovider.h"
    "${CMAKE_SOURCE_DIR}/src/providers/well/idoswelltopsmetadata.h"
    "${CMAKE_SOURCE_DIR}/src/providers/well/idoswelltopsprovider.h"
    "${CMAKE_SOURCE_DIR}/src/render/idos_render.h"
    "${CMAKE_SOURCE_DIR}/src/render/tools/idosrenderinteractiontool.h"
    "${CMAKE_SOURCE_DIR}/src/render/idosrenderobject.h"
    "${CMAKE_SOURCE_DIR}/src/app/render/idosrenderproviderregistry.h"
    "${CMAKE_SOURCE_DIR}/src/render/idosrenderscene.h"
    "${CMAKE_SOURCE_DIR}/src/app/render/idosrenderserver.h"
    "${CMAKE_SOURCE_DIR}/src/render/idosrendertypes.h"
    "${CMAKE_SOURCE_DIR}/src/render/idosrenderview.h"
    "${CMAKE_SOURCE_DIR}/src/render/tools/idostool.h"
    "${CMAKE_SOURCE_DIR}/src/render/tools/idostoolbar.h"
    "${CMAKE_SOURCE_DIR}/src/render/tools/idostoolproviderregistry.h"
    "${CMAKE_SOURCE_DIR}/src/app/render/idosrenderprovidermetadata.h"
    "${CMAKE_SOURCE_DIR}/src/render/idosrenderprovider.h"
    "${CMAKE_SOURCE_DIR}/src/render/tools/idostoolprovidermetadata.h"
    "${CMAKE_SOURCE_DIR}/src/render/tools/idostoolprovider.h"
)
list(APPEND _idos_render_headers
    "${CMAKE_SOURCE_DIR}/src/app/command/idoslistobjectscommand.h"
    "${CMAKE_SOURCE_DIR}/src/app/command/idosinspectobjectcommand.h"
    "${CMAKE_SOURCE_DIR}/src/core/tree/idosdatatreeitemmodel.h"
    "${CMAKE_SOURCE_DIR}/src/core/tree/idoscasetreeitemmodel.h"
    "${CMAKE_SOURCE_DIR}/src/gui/tree/idosdatatreeview.h"
    "${CMAKE_SOURCE_DIR}/src/gui/tree/idoscasetreeview.h"
    "${CMAKE_SOURCE_DIR}/src/plugins/idosfracturemodelingplugin/idosdfngeneratecommand.h"
    "${CMAKE_SOURCE_DIR}/src/plugins/idosfracturemodelingplugin/idosdfngenerationwidget.h"
    "${CMAKE_SOURCE_DIR}/src/plugins/idosfracturemodelingplugin/idosdfngenerator.h"
    "${CMAKE_SOURCE_DIR}/src/gui/idoswaitingdialog.h"
    "${CMAKE_SOURCE_DIR}/src/app/command/idosdeletedatacommand.h"
)
set(_idos_render_header_sources)
foreach(_idos_header IN LISTS _idos_render_headers)
    get_filename_component(_idos_header_name "${_idos_header}" NAME)
    set(_idos_header_source "${CMAKE_CURRENT_BINARY_DIR}/header_${_idos_header_name}.cpp")
    file(WRITE "${_idos_header_source}" "#include \"${_idos_header}\"\n")
    list(APPEND _idos_render_header_sources "${_idos_header_source}")
endforeach()
add_library(idos_render_header_checks OBJECT ${_idos_render_header_sources})
target_link_libraries(idos_render_header_checks PRIVATE idos_app idos_assistant)
target_include_directories(idos_render_header_checks PRIVATE "${CMAKE_SOURCE_DIR}/src/plugins")
add_executable(idos_command_registry_tests command_registry_test.h command_registry_test.cpp)
target_link_libraries(idos_command_registry_tests PRIVATE idos_app ADS::ADS Qt5::Test VTK::CommonCore VTK::hdf5)
add_test(NAME command_registry COMMAND idos_command_registry_tests)
set_tests_properties(command_registry PROPERTIES
    TIMEOUT 30
    ENVIRONMENT "IDOS_DISABLE_UI_SETTINGS=1;IDOS_DISABLE_ASSISTANT_AUTOSTART=1;QT_QPA_PLATFORM=windows;QT_QPA_PLATFORM_PLUGIN_PATH=${IDOS_QT_DIR}/plugins/platforms;IDOS_TEST_SCREENSHOT_DIR=${CMAKE_CURRENT_BINARY_DIR}"
)

add_executable(idos_assistant_widget_tests assistant_widget_test.h assistant_widget_test.cpp)
target_link_libraries(idos_assistant_widget_tests PRIVATE idos_app idos_assistant Qt5::Test)
add_test(NAME assistant_widget COMMAND idos_assistant_widget_tests)
set_tests_properties(assistant_widget PROPERTIES
    TIMEOUT 30
    ENVIRONMENT "IDOS_DISABLE_UI_SETTINGS=1;QT_QPA_PLATFORM=windows;QT_QPA_PLATFORM_PLUGIN_PATH=${IDOS_QT_DIR}/plugins/platforms;IDOS_TEST_SCREENSHOT_DIR=${CMAKE_CURRENT_BINARY_DIR}"
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
    ENVIRONMENT "IDOS_DISABLE_UI_SETTINGS=1;IDOS_DISABLE_ASSISTANT_AUTOSTART=1;QT_QPA_PLATFORM=windows;QT_QPA_PLATFORM_PLUGIN_PATH=${IDOS_QT_DIR}/plugins/platforms;IDOS_TEST_SCREENSHOT_DIR=${CMAKE_CURRENT_BINARY_DIR}"
)

add_executable(idos_well_import_tests well_import_test.h well_import_test.cpp)
target_link_libraries(idos_well_import_tests PRIVATE idos_app Qt5::Test QXlsx::QXlsx)
add_test(NAME well_import COMMAND idos_well_import_tests)
set_tests_properties(well_import PROPERTIES
    TIMEOUT 30
    ENVIRONMENT "IDOS_DISABLE_UI_SETTINGS=1;QT_QPA_PLATFORM=windows;QT_QPA_PLATFORM_PLUGIN_PATH=${IDOS_QT_DIR}/plugins/platforms;IDOS_TEST_SCREENSHOT_DIR=${CMAKE_CURRENT_BINARY_DIR}"
)

add_executable(idos_well_path_tests well_path_test.h well_path_test.cpp)
target_link_libraries(idos_well_path_tests PRIVATE idos_app Qt5::Test)
add_dependencies(idos_well_path_tests idos_project_tests)
add_test(NAME well_path COMMAND idos_well_path_tests)
set_tests_properties(well_path PROPERTIES
    TIMEOUT 30
    ENVIRONMENT "IDOS_DISABLE_UI_SETTINGS=1;QT_QPA_PLATFORM=windows;QT_QPA_PLATFORM_PLUGIN_PATH=${IDOS_QT_DIR}/plugins/platforms;IDOS_TEST_SCREENSHOT_DIR=${CMAKE_CURRENT_BINARY_DIR}"
)

add_executable(idos_seismic_import_tests seismic_import_test.h seismic_import_test.cpp)
target_link_libraries(idos_seismic_import_tests PRIVATE idos_app Qt5::Test VTK::CommonCore VTK::CommonDataModel VTK::RenderingCore)
add_test(NAME seismic_import COMMAND idos_seismic_import_tests
    -o "${CMAKE_CURRENT_BINARY_DIR}/seismic_import.txt,txt" -o -,txt)
set_tests_properties(seismic_import PROPERTIES
    TIMEOUT 60
    ENVIRONMENT "IDOS_DISABLE_UI_SETTINGS=1;QT_QPA_PLATFORM=windows;QT_QPA_PLATFORM_PLUGIN_PATH=${IDOS_QT_DIR}/plugins/platforms;IDOS_TEST_SCREENSHOT_DIR=${CMAKE_CURRENT_BINARY_DIR}"
)

add_executable(idos_case_tests case_workflow_test.h case_workflow_test.cpp)
target_link_libraries(idos_case_tests PRIVATE idos_app Qt5::Test OPM::opmcommon)
if(WIN32)
    target_link_libraries(idos_case_tests PRIVATE Psapi)
endif()
target_link_libraries(idos_case_tests PRIVATE VTK::CommonCore VTK::CommonDataModel VTK::RenderingCore)
target_link_libraries(idos_case_tests PRIVATE VTK::FiltersCore VTK::GUISupportQt)
add_dependencies(idos_case_tests idos_project_tests)
add_dependencies(idos_case_tests idosexampleplugin)
add_dependencies(idos_case_tests idosfracturemodelingplugin idosmultiscalefractureplugin)
add_test(NAME case_workflow COMMAND idos_case_tests
    -o "${CMAKE_CURRENT_BINARY_DIR}/case_workflow.txt,txt" -o -,txt)
set_tests_properties(case_workflow PROPERTIES
    TIMEOUT 240
    ENVIRONMENT "IDOS_DISABLE_UI_SETTINGS=1;QT_QPA_PLATFORM=windows;QT_QPA_PLATFORM_PLUGIN_PATH=${IDOS_QT_DIR}/plugins/platforms;IDOS_TEST_SCREENSHOT_DIR=${CMAKE_CURRENT_BINARY_DIR};IDOS_DISABLE_ASSISTANT_AUTOSTART=1"
)

add_executable(idos_tree_incremental_tests tree_incremental_test.h tree_incremental_test.cpp)
target_link_libraries(idos_tree_incremental_tests PRIVATE idos_app Qt5::Test VTK::CommonCore VTK::GUISupportQt)
add_dependencies(idos_tree_incremental_tests idos_project_tests)
add_test(NAME tree_incremental COMMAND idos_tree_incremental_tests)
set_tests_properties(tree_incremental PROPERTIES
    TIMEOUT 30
    ENVIRONMENT "IDOS_DISABLE_UI_SETTINGS=1;QT_QPA_PLATFORM=windows;QT_QPA_PLATFORM_PLUGIN_PATH=${IDOS_QT_DIR}/plugins/platforms;IDOS_TEST_SCREENSHOT_DIR=${CMAKE_CURRENT_BINARY_DIR};IDOS_DISABLE_ASSISTANT_AUTOSTART=1"
)
