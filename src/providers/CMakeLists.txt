# idos_providers: 数据解析 provider 框架 + 具体实现（LAS / ECLIPSE / ROFF ...）
# 依赖链：idos_providers → idos_core → (OPM 静态库)
add_library(idos_providers SHARED
    idosprovidermetadata.cpp
    idosproviderregistry.cpp
    idosdataloadservice.cpp
    well/idoswellheadermetadata.cpp
    well/idoswellheaderprovider.cpp
    well/idoswelllasmetadata.cpp
    well/idoswelllasprovider.cpp
    grid/idosgrideclipsemetadata.cpp
    grid/idosgrideclipseprovider.cpp
    case/idossimulationcaseeclipsemetadata.cpp
    case/idossimulationcaseeclipseprovider.cpp
)
target_include_directories(idos_providers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# OPM 静态库：只在 providers 模块用（Parser 解析 .DATA）；不对外暴露
target_link_libraries(idos_providers PUBLIC idos_core PRIVATE OPM::opmcommon)
target_compile_definitions(idos_providers PRIVATE IDOS_PROVIDERS_LIB)
