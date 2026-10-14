# idos_providers: 数据解析 provider 框架 + 具体实现（LAS / ECLIPSE / ROFF ...）
# 依赖链：idos_providers → idos_core → (OPM 静态库)
add_library(idos_providers SHARED
    seismic/idosmicroseismicprovider.h
    seismic/idosmicroseismicprovider.cpp
    seismic/idosmicroseismicmetadata.h
    seismic/idosmicroseismicmetadata.cpp
    seismic/idossegyprovider.h
    seismic/idossegyprovider.cpp
    seismic/idossegyprovidermetadata.h
    seismic/idossegyprovidermetadata.cpp
    fracture/idosfracturescheduleprovider.h
    fracture/idosfracturescheduleprovider.cpp
    fracture/idosfractureschedulemetadata.h
    fracture/idosfractureschedulemetadata.cpp
    fracture/idosnaturalfractureprovider.h
    fracture/idosnaturalfractureprovider.cpp
    fracture/idosnaturalfracturemetadata.h
    fracture/idosnaturalfracturemetadata.cpp
    production/idosproductionhistoryprovider.h
    production/idosproductionhistoryprovider.cpp
    production/idosproductionhistorymetadata.h
    production/idosproductionhistorymetadata.cpp
    fracture/idosfracturematerialcatalogprovider.h
    fracture/idosfracturematerialcatalogprovider.cpp
    fracture/idosfracturematerialcatalogmetadata.h
    fracture/idosfracturematerialcatalogmetadata.cpp
    well/idoswelltopsprovider.cpp
    well/idoswelltopsmetadata.cpp
    well/idoswellcompletionprovider.cpp
    well/idoswellcompletionmetadata.cpp
    well/idoswellcheckshotprovider.cpp
    well/idoswellcheckshotmetadata.cpp
    idosdataprovider.cpp
    idosdataprovidermetadata.cpp
    idosdataproviderregistry.cpp
    well/idoswellheadermetadata.cpp
    well/idoswellheaderprovider.cpp
    well/idoswellpathprovider.cpp
    well/idoswellpathmetadata.cpp
    las/idoswelllasmetadata.cpp
    las/idoswelllasprovider.cpp
    fluidrock/idosfluidrocktextprovider.cpp
    grid/idosgridpropertyeclipsemetadata.cpp
    grid/idosgridpropertyeclipseprovider.cpp
    grid/idosgridsurfaceprovider.h
    grid/idosgridsurfaceprovider.cpp
    grid/idosgridsurfacemetadata.h
    grid/idosgridsurfacemetadata.cpp
    grid/idosgridboundaryprovider.h
    grid/idosgridboundaryprovider.cpp
    grid/idosgridboundarymetadata.h
    grid/idosgridboundarymetadata.cpp
    grid/idosgrideclipsemetadata.cpp
    grid/idosgrideclipseprovider.cpp
    case/idossimulationcaseeclipsemetadata.cpp
    case/idossimulationcaseeclipseprovider.cpp
)
target_include_directories(idos_providers PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/fracture
    ${CMAKE_CURRENT_SOURCE_DIR}/production
    ${CMAKE_CURRENT_SOURCE_DIR}/seismic
)
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
