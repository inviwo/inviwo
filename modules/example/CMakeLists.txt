#--------------------------------------------------------------------
# Inviwo Example Module
ivw_module(Example)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/exampleprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/exampleprogressbar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/simpleraycaster.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/exampleprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/exampleprogressbar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/simpleraycaster.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/simpleraycasting.frag
)
ivw_group("Shader Files" ${SHADER_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})

#--------------------------------------------------------------------
# Package or build shaders into resources
ivw_handle_shader_resources(${CMAKE_CURRENT_SOURCE_DIR}/glsl ${SHADER_FILES})