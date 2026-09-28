# idos_core: business data and type registry
# Dependency chain: no custom module dependencies, only Qt5::Core and Qt5::Gui
add_library(idos_core SHARED
    idos_core.h
    idoscaseitemref.h
    idoscaseitemref.cpp
    idoscaseobject.h
    idoscaseobject.cpp
    idosdataobject.h
    idosdataobject.cpp
    idosgrid.h
    idosgrid.cpp
    idosgridcell.h
    idosgridcell.cpp
    idosgridproperty.h
    idosgridproperty.cpp
    idosobject.h
    idosobject.cpp
    idosproject.h
    idosproject.cpp
    idosprojectmetadata.h
    idosprojectmetadata.cpp
    idossimulationcaseobject.h
    idossimulationcaseobject.cpp
    idostyperegistry.h
    idostyperegistry.cpp
    idoswell.h
    idoswell.cpp
    idoswellcompletion.h
    idoswellcompletion.cpp
    idoswellcontrol.h
    idoswellcontrol.cpp
    idoswellgeometryresolver.h
    idoswellgeometryresolver.cpp
    idoswellhead.h
    idoswellhead.cpp
    idoswelllogchannel.h
    idoswelllogchannel.cpp
    idoswelllogset.h
    idoswelllogset.cpp
    idoswellmarker.h
    idoswellmarker.cpp
    idoswellmarkerset.h
    idoswellmarkerset.cpp
    idoswellpath.h
    idoswellpath.cpp
    idoswellpathpoint.h
    idoswellpathpoint.cpp
    idoswellsegment.h
    idoswellsegment.cpp
    idoswellspatialreference.h
    idoswellspatialreference.cpp
)
target_include_directories(idos_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(idos_core PUBLIC Qt5::Core Qt5::Gui)
target_compile_definitions(idos_core PRIVATE IDOS_CORE_LIB)

get_target_property(_idos_sources idos_core SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_idos_sources})
