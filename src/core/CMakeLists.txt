# idos_core: business data and type registry
# Dependency chain: no custom module dependencies; Qt Widgets is required by the tree model.
add_library(idos_core SHARED
    data/case/idoscaseitemref.cpp
    data/case/idoscaseitemref.h
    data/case/idoscaseobject.cpp
    data/case/idoscaseobject.h
    data/case/idossimulationcasemetadata.cpp
    data/case/idossimulationcasemetadata.h
    data/case/idossimulationcaseobject.cpp
    data/case/idossimulationcaseobject.h
    data/case/idossimulationsummaryresultsobject.cpp
    data/case/idossimulationsummaryresultsobject.h
    data/case/idossimulationsummaryresultsmetadata.cpp
    data/case/idossimulationsummaryresultsmetadata.h
    data/fluidrock/idosfluidrocktableobject.cpp
    data/fluidrock/idosfluidrocktableobject.h
    data/fluidrock/idosfluidrocktablemetadata.cpp
    data/fluidrock/idosfluidrocktablemetadata.h
    data/fracture/idosfracturematerialcatalogobject.cpp
    data/fracture/idosfracturematerialcatalogobject.h
    data/fracture/idosfracturematerialcatalogdatametadata.cpp
    data/fracture/idosfracturematerialcatalogdatametadata.h
    data/fracture/idosfracturescheduleobject.cpp
    data/fracture/idosfracturescheduleobject.h
    data/fracture/idosfracturescheduledatametadata.cpp
    data/fracture/idosfracturescheduledatametadata.h
    data/fracture/idosnaturalfracturesetobject.cpp
    data/fracture/idosnaturalfracturesetobject.h
    data/fracture/idosnaturalfracturesetdatametadata.cpp
    data/fracture/idosnaturalfracturesetdatametadata.h
    data/grid/idosgridobject.cpp
    data/grid/idosgridobject.h
    data/grid/idosgridboundaryobject.cpp
    data/grid/idosgridboundaryobject.h
    data/grid/idosgridboundarydatametadata.cpp
    data/grid/idosgridboundarydatametadata.h
    data/grid/idosgridcell.cpp
    data/grid/idosgridcell.h
    data/grid/idosgridfilterobject.cpp
    data/grid/idosgridfilterobject.h
    data/grid/idosgridfiltermetadata.cpp
    data/grid/idosgridfiltermetadata.h
    data/grid/idosgridmetadata.cpp
    data/grid/idosgridmetadata.h
    data/grid/idosgridpropertymetadata.cpp
    data/grid/idosgridpropertymetadata.h
    data/grid/idosgridpropertyobject.cpp
    data/grid/idosgridpropertyobject.h
    data/grid/idosgridsurfaceobject.cpp
    data/grid/idosgridsurfaceobject.h
    data/grid/idosgridsurfacedatametadata.cpp
    data/grid/idosgridsurfacedatametadata.h
    data/production/idosproductionhistoryobject.cpp
    data/production/idosproductionhistoryobject.h
    data/production/idosproductionhistorydatametadata.cpp
    data/production/idosproductionhistorydatametadata.h
    data/seismic/idosmicroseismicdatametadata.cpp
    data/seismic/idosmicroseismicdatametadata.h
    data/seismic/idosmicroseismicsetobject.cpp
    data/seismic/idosmicroseismicsetobject.h
    data/seismic/idosseismicvolumeobject.cpp
    data/seismic/idosseismicvolumeobject.h
    data/seismic/idosseismicvolumedatametadata.cpp
    data/seismic/idosseismicvolumedatametadata.h
    data/well/idoswellobject.cpp
    data/well/idoswellobject.h
    data/well/idoswellcheckshot.cpp
    data/well/idoswellcheckshot.h
    data/well/idoswellcompletion.cpp
    data/well/idoswellcompletion.h
    data/well/idoswellcontrol.cpp
    data/well/idoswellcontrol.h
    data/well/idoswellgeometryresolver.cpp
    data/well/idoswellgeometryresolver.h
    data/well/idoswellhead.cpp
    data/well/idoswellhead.h
    data/well/idoswelllogchannel.cpp
    data/well/idoswelllogchannel.h
    data/well/idoswelllogset.cpp
    data/well/idoswelllogset.h
    data/well/idoswellmarker.cpp
    data/well/idoswellmarker.h
    data/well/idoswellmarkerset.cpp
    data/well/idoswellmarkerset.h
    data/well/idoswellmetadata.cpp
    data/well/idoswellmetadata.h
    data/well/idoswellpath.cpp
    data/well/idoswellpath.h
    data/well/idoswellpathpoint.cpp
    data/well/idoswellpathpoint.h
    data/well/idoswellsegment.cpp
    data/well/idoswellsegment.h
    data/well/idoswellspatialreference.cpp
    data/well/idoswellspatialreference.h
    idos_core.h
    command/idoscommand.cpp
    command/idoscommand.h
    command/idoscommandmanager.cpp
    command/idoscommandmanager.h
    command/idoscommandmetadata.cpp
    command/idoscommandmetadata.h
    command/idoscommandregistry.cpp
    command/idoscommandregistry.h
    idosdataobject.cpp
    idosdataobject.h
    idosobject.cpp
    idosobject.h
    idosproject.cpp
    idosproject.h
    idosprojectmetadata.cpp
    idosprojectmetadata.h
    logger/idosconsoletarget.cpp
    logger/idosconsoletarget.h
    logger/idosfiletarget.cpp
    logger/idosfiletarget.h
    logger/idoslogger.cpp
    logger/idoslogger.h
    logger/idosloglevel.cpp
    logger/idosloglevel.h
    logger/idoslogrecord.cpp
    logger/idoslogrecord.h
    logger/idoslogtarget.h
    logger/idosmsvctarget.cpp
    logger/idosmsvctarget.h
    data/idosdatametadata.cpp
    data/idosdatametadata.h
    providers/idosdataprovider.cpp
    providers/idosdataprovider.h
    providers/idosdataprovidermetadata.cpp
    providers/idosdataprovidermetadata.h
    providers/idosdataproviderregistry.cpp
    providers/idosdataproviderregistry.h
    data/idosdataregistry.cpp
    data/idosdataregistry.h
    tree/idostreeitemprovidermetadata.cpp
    tree/idostreeitemprovidermetadata.h
    tree/idostreeitemprovider.cpp
    tree/idostreeitemprovider.h
    tree/idostreeitemproviderregistry.cpp
    tree/idostreeitemproviderregistry.h
    tree/idostreeaccess.cpp
    tree/idostreeaccess.h
    tree/idostreemodel.h
    tree/idostreeitem.cpp
    tree/idostreeitem.h
    tree/idosdatatreeitemmodel.cpp
    tree/idosdatatreeitemmodel.h
    tree/idoscasetreeitemmodel.cpp
    tree/idoscasetreeitemmodel.h
    tree/idostreemodel.cpp
)
target_include_directories(idos_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(idos_core PUBLIC Qt5::Core Qt5::Gui Qt5::Widgets)
target_link_libraries(idos_core PRIVATE VTK::hdf5)
target_link_libraries(idos_core PRIVATE PROJ::proj)
add_custom_command(TARGET idos_core POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:PROJ::proj>" "$<TARGET_FILE_DIR:idos_core>"
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:idos_core>/resources/proj"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/resources/proj"
        "$<TARGET_FILE_DIR:idos_core>/resources/proj"
    VERBATIM
)

target_compile_definitions(idos_core PRIVATE IDOS_CORE_LIB)

get_target_property(_idos_sources idos_core SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_idos_sources})
