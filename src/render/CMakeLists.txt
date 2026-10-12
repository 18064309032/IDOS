add_library(idos_render SHARED
    idostool.h
    idostool.cpp
    idostoolmetadata.h
    idostoolmetadata.cpp
    idostoolregistry.h
    idostoolregistry.cpp
    idostoolprovider.h
    idostoolprovider.cpp
    idostoolbar.h
    idostoolbar.cpp
    idosrenderserver.h
    idosrenderserver.cpp
    idosrendermetadata.h
    idosrendermetadata.cpp
    idosrenderregistry.h
    idosrenderregistry.cpp
    idos_render.h
    idosrenderobject.h
    idosrenderobject.cpp
    idosrenderprovider.h
    idosrenderprovider.cpp
    idosrenderscene.h
    idosrenderscene.cpp
    idosrendertypes.h
    idosrendertypes.cpp
    idosrenderview.h
    idosrenderview.cpp
)

target_include_directories(idos_render
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(idos_render PUBLIC idos_core Qt5::Core Qt5::Gui Qt5::Widgets)
target_compile_definitions(idos_render PRIVATE IDOS_RENDER_LIB)

if(IDOS_ENABLE_RENDER)
    target_compile_definitions(idos_render PUBLIC IDOS_ENABLE_RENDER)
    target_include_directories(idos_render PUBLIC
        "${CMAKE_SOURCE_DIR}/thirdparty/vtk9.6.0/include/vtk-9.6"
    )
    target_link_libraries(idos_render PRIVATE
        VTK::CommonCore
        VTK::CommonDataModel
        VTK::RenderingCore
        VTK::RenderingOpenGL2
        VTK::InteractionStyle
        VTK::InteractionWidgets
        VTK::RenderingAnnotation
        VTK::GUISupportQt
    )
    vtk_module_autoinit(
        TARGETS idos_render
        MODULES ${VTK_LIBRARIES}
    )
endif()

get_target_property(_idos_sources idos_render SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${_idos_sources})
