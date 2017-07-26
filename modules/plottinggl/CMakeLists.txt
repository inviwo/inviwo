#--------------------------------------------------------------------
# Inviwo PlottingGL Module
ivw_module(PlottingGL)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/plotters/scatterplotgl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/axisrenderprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/parallelcoordinates.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/scatterplotmatrixprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/scatterplotprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeaxis.h
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/axisrenderer.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/plotters/scatterplotgl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/axisrenderprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/parallelcoordinates.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/scatterplotmatrixprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/scatterplotprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeaxis.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/axisrenderer.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/pcp_axis.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/pcp_axis.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/pcp_common.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/pcp_handle.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/pcp_handle.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/pcp_lines.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/pcp_lines.geom
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/pcp_lines.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/plotting/common.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/scatterplot.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/scatterplot.geom
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/scatterplot.vert
)
ivw_group("Shader Files" ${SHADER_FILES})

#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
    #${CMAKE_CURRENT_SOURCE_DIR}/tests/plottinggl-test.cpp
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

#--------------------------------------------------------------------
# Add shader directory to pack
ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)
