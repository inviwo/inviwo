if(IVW_MODULE_FONTRENDERING)
    add_subdirectory(ext/freetype)
endif(IVW_MODULE_FONTRENDERING)

#--------------------------------------------------------------------
# Inviwo FontRendering Module
ivw_module(FontRendering)

#--------------------------------------------------------------------
# Set include directories
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext/freetype/include)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/fontrenderingmodule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/fontrenderingmoduledefine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/textoverlaygl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/textrenderer.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/fontrenderingmodule.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/textoverlaygl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/textrenderer.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/fontrendering_freetype.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/fontrendering_freetype.vert
)
ivw_group("Shader Files" ${SHADER_FILES})

#--------------------------------------------------------------------
# Disable Pre-compiled headers as file is not generated properly on CMake 3.1 ->
ivw_set_pch_disabled_for_module()

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

#--------------------------------------------------------------------
# Package or build shaders into resources
ivw_handle_shader_resources(${CMAKE_CURRENT_SOURCE_DIR}/glsl ${SHADER_FILES})

#--------------------------------------------------------------------
# Add font directory to pack
ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/fonts)