add_subdirectory(ext/freetype)

#--------------------------------------------------------------------
# Inviwo FontRendering Module
ivw_module(FontRendering)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/modules/fontrendering/datastructures/fontdata.h
    include/modules/fontrendering/datastructures/fontsettings.h
    include/modules/fontrendering/datastructures/texatlasentry.h
    include/modules/fontrendering/datastructures/textboundingbox.h
    include/modules/fontrendering/fontrenderingmodule.h
    include/modules/fontrendering/fontrenderingmoduledefine.h
    include/modules/fontrendering/processors/textoverlaygl.h
    include/modules/fontrendering/properties/fontproperty.h
    include/modules/fontrendering/textrenderer.h
    include/modules/fontrendering/util/fontutils.h
    include/modules/fontrendering/util/textureatlas.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/datastructures/fontdata.cpp
    src/datastructures/fontsettings.cpp
    src/datastructures/texatlasentry.cpp
    src/datastructures/textboundingbox.cpp
    src/fontrenderingmodule.cpp
    src/processors/textoverlaygl.cpp
    src/properties/fontproperty.cpp
    src/textrenderer.cpp
    src/util/fontutils.cpp
    src/util/textureatlas.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/fontrendering_freetype.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/fontrendering_freetype.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/textrenderer.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/textrenderer.vert
)
ivw_group("Shader Files" ${SHADER_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

#--------------------------------------------------------------------
# Package or build shaders into resources
ivw_handle_shader_resources(${CMAKE_CURRENT_SOURCE_DIR}/glsl ${SHADER_FILES})

#--------------------------------------------------------------------
# Add font directory to pack
ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/fonts)

#--------------------------------------------------------------------
# register license files
ivw_register_license_file(NAME FreeType MODULE FontRendering
    URL "https://www.freetype.org/"
    TYPE "BSD-style, acknowledgement: Portions of this software are copyright The FreeType Project (www.freetype.org). All rights reserved."
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/freetype/freetype/docs/FTL.txt
)

#--------------------------------------------------------------------
# Add dependency to ext/utf 
target_link_libraries(inviwo-module-fontrendering PRIVATE utf)
target_link_libraries(inviwo-module-fontrendering PUBLIC freetype)
ivw_make_package(InviwoFontRenderingModule inviwo-module-fontrendering)
