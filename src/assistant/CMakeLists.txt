# idos_assistant: local model process and HTTP service
add_library(idos_assistant SHARED
    idos_assistant.h
    idosassistantwidget.h
    idosassistantwidget.cpp
    idoslocalmodelprocess.h
    idoslocalmodelprocess.cpp
    idoslocalmodelservice.h
    idoslocalmodelservice.cpp
    idoslocalassistantservice.h
    idoslocalassistantservice.cpp
)

target_include_directories(idos_assistant PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(idos_assistant PUBLIC
    idos_core
    Qt5::Core
    Qt5::Network
    Qt5::Widgets
)
target_compile_definitions(idos_assistant PRIVATE IDOS_ASSISTANT_LIB)

get_target_property(_idos_sources idos_assistant SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_idos_sources})
