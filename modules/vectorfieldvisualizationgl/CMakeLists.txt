#--------------------------------------------------------------------
# Inviwo VectorFieldVisualizationGL Module
ivw_module(VectorFieldVisualizationGL)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/2d/hedgehog2d.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/2d/lic2d.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/2d/vector2dcurl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/2d/vector2ddivergence.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/2d/vector2dmagnitude.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/3d/vector3dcurl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/3d/vector3ddivergence.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/datageneration/lorenzsystem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/datageneration/vectorfieldgenerator2d.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/datageneration/vectorfieldgenerator3d.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/2d/hedgehog2d.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/2d/lic2d.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/2d/vector2dcurl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/2d/vector2ddivergence.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/2d/vector2dmagnitude.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/3d/vector3dcurl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/3d/vector3ddivergence.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/datageneration/lorenzsystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/datageneration/vectorfieldgenerator2d.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/datageneration/vectorfieldgenerator3d.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/lic2d.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/lorenzsystem.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/streamlinesgl.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/streamlinesgl.geom
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/streamlinesgl.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/vector2dcurl.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/vector2ddivergence.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/vector2dmagnitude.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/vector3dcurl.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/vector3ddivergence.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/vectorfieldgenerator2d.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/vectorfieldgenerator3d.frag
)
ivw_group("Shader Files" ${SHADER_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

#--------------------------------------------------------------------
# Add shader directory to pack
ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)