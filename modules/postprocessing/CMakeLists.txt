#--------------------------------------------------------------------
# Inviwo PostProcessing Module
ivw_module(PostProcessing)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/ssao.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/fxaa.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/ssao.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/fxaa.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/common.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/bilateralblur.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/depthlinearize.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/displaytex.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/fullscreenquad.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/fullscreenquad.geom
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/hbao.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/hbao_blur.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/hbao_deinterleave.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/hbao_reinterleave.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/viewnormal.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/Fxaa3_11.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/fxaa.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/rgbl.frag
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