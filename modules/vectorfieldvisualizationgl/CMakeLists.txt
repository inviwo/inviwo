#--------------------------------------------------------------------
# Inviwo VectorFieldVisualizationGL Module
ivw_module(VectorFieldVisualizationGL)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/datageneration/lorenzsystem.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/datageneration/lorenzsystem.cpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/vectorfieldgenerator.frag
)
ivw_group("Shader Files" ${SHADER_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

#--------------------------------------------------------------------
# Add shader directory to pack
ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)