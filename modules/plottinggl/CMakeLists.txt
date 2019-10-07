#--------------------------------------------------------------------
# Inviwo PlottingGL Module
ivw_module(PlottingGL)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/modules/plottinggl/datavisualizer/pcpdataframevisualizer.h
    include/modules/plottinggl/datavisualizer/scatterplotdataframevisualizer.h
    include/modules/plottinggl/plotters/persistencediagramplotgl.h
    include/modules/plottinggl/plotters/scatterplotgl.h
    include/modules/plottinggl/plottingglmodule.h
    include/modules/plottinggl/plottingglmoduledefine.h
    include/modules/plottinggl/processors/axisrenderprocessor.h
    include/modules/plottinggl/processors/colorscalelegend.h
    include/modules/plottinggl/processors/imageplotprocessor.h
    include/modules/plottinggl/processors/parallelcoordinates/parallelcoordinates.h
    include/modules/plottinggl/processors/parallelcoordinates/pcpaxissettings.h
    include/modules/plottinggl/processors/persistencediagramplotprocessor.h
    include/modules/plottinggl/processors/scatterplotmatrixprocessor.h
    include/modules/plottinggl/processors/scatterplotprocessor.h
    include/modules/plottinggl/processors/volumeaxis.h
    include/modules/plottinggl/utils/axisrenderer.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/datavisualizer/pcpdataframevisualizer.cpp
    src/datavisualizer/scatterplotdataframevisualizer.cpp
    src/plotters/persistencediagramplotgl.cpp
    src/plotters/scatterplotgl.cpp
    src/plottingglmodule.cpp
    src/processors/axisrenderprocessor.cpp
    src/processors/colorscalelegend.cpp
    src/processors/imageplotprocessor.cpp
    src/processors/parallelcoordinates/parallelcoordinates.cpp
    src/processors/parallelcoordinates/pcpaxissettings.cpp
    src/processors/persistencediagramplotprocessor.cpp
    src/processors/scatterplotmatrixprocessor.cpp
    src/processors/scatterplotprocessor.cpp
    src/processors/volumeaxis.cpp
    src/utils/axisrenderer.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/legend.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/pcp_common.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/pcp_lines.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/pcp_lines.geom
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/pcp_lines.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/plotting/common.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/persistencediagram.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/persistencediagramlines.vert
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
