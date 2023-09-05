#--------------------------------------------------------------------
# Inviwo PostProcessing Module
ivw_module(PostProcessing)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/modules/postprocessing/postprocessingmodule.h
    include/modules/postprocessing/postprocessingmoduledefine.h
    include/modules/postprocessing/processors/depthdarkening.h
    include/modules/postprocessing/processors/depthoffield.h
    include/modules/postprocessing/processors/fog.h
    include/modules/postprocessing/processors/fxaa.h
    include/modules/postprocessing/processors/hdrbloom.h
    include/modules/postprocessing/processors/imagebrightnesscontrast.h
    include/modules/postprocessing/processors/imageedgedarken.h
    include/modules/postprocessing/processors/imagefilter.h
    include/modules/postprocessing/processors/imagehuesaturationluminance.h
    include/modules/postprocessing/processors/imageopacity.h
    include/modules/postprocessing/processors/ssao.h
    include/modules/postprocessing/processors/tonemapping.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/postprocessingmodule.cpp
    src/processors/depthdarkening.cpp
    src/processors/depthoffield.cpp
    src/processors/fog.cpp
    src/processors/fxaa.cpp
    src/processors/hdrbloom.cpp
    src/processors/imagebrightnesscontrast.cpp
    src/processors/imageedgedarken.cpp
    src/processors/imagefilter.cpp
    src/processors/imagehuesaturationluminance.cpp
    src/processors/imageopacity.cpp
    src/processors/ssao.cpp
    src/processors/tonemapping.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/bilateralblur.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/bloomblur.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/bloomcompose.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/bloomhighpass.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/brightnesscontrast.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/colortools.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/common.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/depthdarkening.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/depthlinearize.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/displaytex.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/dof_approx.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/dof_approx.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/dof_exact.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/edgedarken.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/fog.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/fog.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/fullscreenquad.geom
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/fullscreenquad.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/fxaa.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/Fxaa3_11.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/hbao.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/hbao_blur.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/hbao_deinterleave.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/hbao_reinterleave.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/huesaturationluminance.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/imageopacity.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/rgbl.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/tonemapping.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/viewnormal.frag
)
ivw_group("Shader Files" ${SHADER_FILES})


#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
    #${CMAKE_CURRENT_SOURCE_DIR}/tests/postprocessing-test.cpp
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

#--------------------------------------------------------------------
# Add shader directory to pack
ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)
