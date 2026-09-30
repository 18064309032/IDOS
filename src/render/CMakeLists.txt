add_library(idos_render SHARED
    idos_render.h
    idosrenderobject.h
    idosrenderobject.cpp
    grid/idosrendermesh.h
    grid/idosrendermesh.cpp
    well/idoswellrenderobject.h
    well/idoswellrenderobject.cpp
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
        ${CMAKE_CURRENT_SOURCE_DIR}/grid
        ${CMAKE_CURRENT_SOURCE_DIR}/well
)
target_link_libraries(idos_render PUBLIC Qt5::Core Qt5::Gui Qt5::Widgets)
target_compile_definitions(idos_render PRIVATE IDOS_RENDER_LIB)

if(IDOS_ENABLE_RENDER)
    target_compile_definitions(idos_render PUBLIC IDOS_ENABLE_RENDER)
    target_include_directories(idos_render PRIVATE
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
