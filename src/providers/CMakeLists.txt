# idos_providers: concrete file format implementations; interfaces live in idos_core.
# 依赖链：idos_providers → idos_core → (OPM 静态库)
add_library(idos_providers SHARED
    case/idossimulationcaseeclipsemetadata.cpp
    case/idossimulationcaseeclipsemetadata.h
    case/idossimulationcaseeclipseprovider.cpp
    case/idossimulationcaseeclipseprovider.h
    fluidrock/idosfluidrocktextprovider.cpp
    fluidrock/idosfluidrocktextprovider.h
    fracture/idosfracturematerialcatalogmetadata.cpp
    fracture/idosfracturematerialcatalogmetadata.h
    fracture/idosfracturematerialcatalogprovider.cpp
    fracture/idosfracturematerialcatalogprovider.h
    fracture/idosfractureschedulemetadata.cpp
    fracture/idosfractureschedulemetadata.h
    fracture/idosfracturescheduleprovider.cpp
    fracture/idosfracturescheduleprovider.h
    fracture/idosnaturalfracturemetadata.cpp
    fracture/idosnaturalfracturemetadata.h
    fracture/idosnaturalfractureprovider.cpp
    fracture/idosnaturalfractureprovider.h
    grid/idosgridboundarymetadata.cpp
    grid/idosgridboundarymetadata.h
    grid/idosgridboundaryprovider.cpp
    grid/idosgridboundaryprovider.h
    grid/idosgrideclipsemetadata.cpp
    grid/idosgrideclipsemetadata.h
    grid/idosgrideclipseprovider.cpp
    grid/idosgrideclipseprovider.h
    grid/idosgridpropertyeclipsemetadata.cpp
    grid/idosgridpropertyeclipsemetadata.h
    grid/idosgridpropertyeclipseprovider.cpp
    grid/idosgridpropertyeclipseprovider.h
    grid/idosgridsurfacemetadata.cpp
    grid/idosgridsurfacemetadata.h
    grid/idosgridsurfaceprovider.cpp
    grid/idosgridsurfaceprovider.h
    idos_providers.h
    las/idoswelllasmetadata.cpp
    las/idoswelllasmetadata.h
    las/idoswelllasprovider.cpp
    las/idoswelllasprovider.h
    production/idosproductionhistorymetadata.cpp
    production/idosproductionhistorymetadata.h
    production/idosproductionhistoryprovider.cpp
    production/idosproductionhistoryprovider.h
    seismic/idosmicroseismicmetadata.cpp
    seismic/idosmicroseismicmetadata.h
    seismic/idosmicroseismicprovider.cpp
    seismic/idosmicroseismicprovider.h
    seismic/idossegyprovider.cpp
    seismic/idossegyprovider.h
    seismic/idossegyprovidermetadata.cpp
    seismic/idossegyprovidermetadata.h
    well/idoswellcheckshotmetadata.cpp
    well/idoswellcheckshotmetadata.h
    well/idoswellcheckshotprovider.cpp
    well/idoswellcheckshotprovider.h
    well/idoswellcompletionmetadata.cpp
    well/idoswellcompletionmetadata.h
    well/idoswellcompletionprovider.cpp
    well/idoswellcompletionprovider.h
    well/idoswellheadermetadata.cpp
    well/idoswellheadermetadata.h
    well/idoswellheaderprovider.cpp
    well/idoswellheaderprovider.h
    well/idoswellpathmetadata.cpp
    well/idoswellpathmetadata.h
    well/idoswellpathprovider.cpp
    well/idoswellpathprovider.h
    well/idoswelltopsmetadata.cpp
    well/idoswelltopsmetadata.h
    well/idoswelltopsprovider.cpp
    well/idoswelltopsprovider.h
)
target_include_directories(idos_providers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# OPM、QXlsx、Shapelib 静态库仅供 providers 模块内部的数据解析使用。
target_link_libraries(idos_providers PUBLIC idos_core PRIVATE
    OPM::opmcommon
    QXlsx::QXlsx
    shapelib::shapelib
)
target_compile_definitions(idos_providers PRIVATE IDOS_PROVIDERS_LIB)
if(MSVC)
    target_compile_definitions(idos_providers PRIVATE _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING)
endif()
