#--------------------------------------------------------------------
# Inviwo Base OpenGL Module
ivw_module(BaseGL)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/background.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/drawlines.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/drawpoints.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/entryexitpoints.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/firstivwprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/meshpicking.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/heightfieldprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imagecompositeprocessorgl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imagelayoutgl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imagemixer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imageoverlaygl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imageprocessing/findedges.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imageprocessing/imagegamma.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imageprocessing/imageglprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imageprocessing/imagegrayscale.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imageprocessing/imageinvert.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imageprocessing/imagemapping.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imageprocessing/imageresample.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/isoraycaster.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/lightingraycaster.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/lightvolumegl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/multichannelraycaster.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/redgreenprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeprocessing/vectormagnitudeprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeprocessing/volumecombiner.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeprocessing/volumediff.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeprocessing/volumeglprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeprocessing/volumegradientmagnitude.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeprocessing/volumegradientprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeprocessing/volumelaplacian.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeprocessing/volumelowpass.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeprocessing/volumemapping.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeraycaster.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeslicegl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/viewmanager.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/background.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/drawlines.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/drawpoints.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/entryexitpoints.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/firstivwprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/meshpicking.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/heightfieldprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imagecompositeprocessorgl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imagelayoutgl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imagemixer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imageoverlaygl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imageprocessing/findedges.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imageprocessing/imagegamma.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imageprocessing/imageglprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imageprocessing/imagegrayscale.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imageprocessing/imageinvert.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imageprocessing/imagemapping.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imageprocessing/imageresample.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/isoraycaster.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/lightingraycaster.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/lightvolumegl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/multichannelraycaster.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/redgreenprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeprocessing/vectormagnitudeprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeprocessing/volumecombiner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeprocessing/volumediff.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeprocessing/volumeglprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeprocessing/volumegradientmagnitude.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeprocessing/volumegradientprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeprocessing/volumelaplacian.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeprocessing/volumelowpass.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeprocessing/volumemapping.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeraycaster.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeslicegl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/viewmanager.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/background.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/capnearclipping.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/heightfield.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/heightfield.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/img_findedges.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/img_gamma.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/img_graysc.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/img_invert.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/img_mapping.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/img_mix.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/img_resample.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/multichannelraycaster.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/volumeslice.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/lighting/lightingraycasting.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/lighting/lightpropagation.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/lighting/lightpropagation.geom
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/lighting/lightpropagation.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/lighting/lightvolumeblend.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/lighting/lightvolumeblend.geom
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/lighting/lightvolumeblend.vert
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

