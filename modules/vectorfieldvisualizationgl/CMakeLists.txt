#--------------------------------------------------------------------
# Inviwo VectorFieldVisualizationGL Module
ivw_module(VectorFieldVisualizationGL)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/modules/vectorfieldvisualizationgl/processors/2d/hedgehog2d.h
    include/modules/vectorfieldvisualizationgl/processors/2d/lic2d.h
    include/modules/vectorfieldvisualizationgl/processors/2d/vector2dcurl.h
    include/modules/vectorfieldvisualizationgl/processors/2d/vector2ddivergence.h
    include/modules/vectorfieldvisualizationgl/processors/2d/vector2dmagnitude.h
    include/modules/vectorfieldvisualizationgl/processors/3d/lic3d.h
    include/modules/vectorfieldvisualizationgl/processors/3d/streamparticles.h
    include/modules/vectorfieldvisualizationgl/processors/3d/vector3dcurl.h
    include/modules/vectorfieldvisualizationgl/processors/3d/vector3ddivergence.h
    include/modules/vectorfieldvisualizationgl/processors/4d/tmip.h
    include/modules/vectorfieldvisualizationgl/processors/datageneration/lorenzsystem.h
    include/modules/vectorfieldvisualizationgl/processors/datageneration/vectorfieldgenerator2d.h
    include/modules/vectorfieldvisualizationgl/processors/datageneration/vectorfieldgenerator3d.h
    include/modules/vectorfieldvisualizationgl/processors/datageneration/vectorfieldgenerator4d.h
    include/modules/vectorfieldvisualizationgl/vectorfieldvisualizationglmodule.h
    include/modules/vectorfieldvisualizationgl/vectorfieldvisualizationglmoduledefine.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/processors/2d/hedgehog2d.cpp
    src/processors/2d/lic2d.cpp
    src/processors/2d/vector2dcurl.cpp
    src/processors/2d/vector2ddivergence.cpp
    src/processors/2d/vector2dmagnitude.cpp
    src/processors/3d/lic3d.cpp
    src/processors/3d/streamparticles.cpp
    src/processors/3d/vector3dcurl.cpp
    src/processors/3d/vector3ddivergence.cpp
    src/processors/4d/tmip.cpp
    src/processors/datageneration/lorenzsystem.cpp
    src/processors/datageneration/vectorfieldgenerator2d.cpp
    src/processors/datageneration/vectorfieldgenerator3d.cpp
    src/processors/datageneration/vectorfieldgenerator4d.cpp
    src/vectorfieldvisualizationglmodule.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/utils/advection.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/lic2d.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/lic3d.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/lorenzsystem.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/streamlinesgl.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/streamlinesgl.geom
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/streamlinesgl.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/streamparticles.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/vector2dcurl.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/vector2ddivergence.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/vector2dmagnitude.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/vector3dcurl.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/vector3ddivergence.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/vectorfieldgenerator2d.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/vectorfieldgenerator3d.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/vectorfieldgenerator4d.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/tmip.frag
)
ivw_group("Shader Files" ${SHADER_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

#--------------------------------------------------------------------
# Package or build shaders into resources
ivw_handle_shader_resources(${CMAKE_CURRENT_SOURCE_DIR}/glsl ${SHADER_FILES})
