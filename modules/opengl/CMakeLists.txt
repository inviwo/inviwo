#--------------------------------------------------------------------
# Inviwo OpenGL Module
ivw_module(OpenGL)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/buffer/buffergl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/buffer/bufferglconverter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/buffer/elementbuffergl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/buffer/elementbufferglconverter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/canvasgl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/canvasprocessorgl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/clockgl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/geometry/meshrenderprocessorgl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/geometry/meshgl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glformats.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glwrap/bufferobject.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glwrap/bufferobjectarray.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glwrap/elementbufferobject.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glwrap/framebufferobject.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glwrap/renderbufferobject.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glwrap/shader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glwrap/shadermanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glwrap/shaderobject.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glwrap/texture.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glwrap/texture2d.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glwrap/texture2darray.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glwrap/texture3d.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glwrap/texturecallback.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glwrap/textureunit.h
    ${CMAKE_CURRENT_SOURCE_DIR}/image/compositeprocessorgl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/image/imagegl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/image/layergl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/image/layerglconverter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/inviwoopengl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/openglcapabilities.h
    ${CMAKE_CURRENT_SOURCE_DIR}/openglexception.h
    ${CMAKE_CURRENT_SOURCE_DIR}/openglmodule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/openglsettings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/openglutils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/rendering/meshdrawergl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shaderutils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/textureobserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/textureutils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/volume/volumegl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/volume/volumeglconverter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/volumeutils.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/buffer/buffergl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/buffer/bufferglconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/buffer/elementbuffergl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/buffer/elementbufferglconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/canvasgl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/canvasprocessorgl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/clockgl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/geometry/meshrenderprocessorgl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/geometry/meshgl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glwrap/bufferobject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glwrap/bufferobjectarray.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glwrap/elementbufferobject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glwrap/framebufferobject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glwrap/renderbufferobject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glwrap/shader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glwrap/shadermanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glwrap/shaderobject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glwrap/texture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glwrap/texture2d.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glwrap/texture2darray.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glwrap/texture3d.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glwrap/textureunit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/image/compositeprocessorgl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/image/imagegl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/image/layergl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/image/layerglconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/inviwoopengl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/openglcapabilities.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/openglexception.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/openglmodule.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/openglsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/openglutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rendering/meshdrawergl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shaderutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/textureutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/volume/volumegl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/volume/volumeglconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/volumeutils.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/composite.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/geometryrendering.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/geometryrendering.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/img_color.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/img_copy.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/img_identity.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/img_noise.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/img_texturequad.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/img_texturequad.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/isoraycasting.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/minimal.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/picking.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/raycasting.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/standard.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/standard.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/utils/classification.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/utils/compositing.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/utils/depth.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/utils/gradients.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/utils/intersection.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/utils/raycastgeometry.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/utils/sampler2d.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/utils/sampler3d.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/utils/shading.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/utils/structs.glsl
)
ivw_group("Shader Files" ${SHADER_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

#--------------------------------------------------------------------
# Handle external resources
if(IVW_USE_GENERATED_RESOURCES)
#--------------------------------------------------------------------
# Generate shader resource (if GENERATE_SHADER_RESOURCES is checked)
ivw_generate_shader_resource(${CMAKE_CURRENT_SOURCE_DIR}/glsl ${SHADER_FILES})
else()
#--------------------------------------------------------------------
# Add shader directory to pack
ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)
endif()

#--------------------------------------------------------------------
# Build GLEW if it couldn't be found
if(BUILD_GLEW)
    add_subdirectory(ext/glew)
endif(BUILD_GLEW)