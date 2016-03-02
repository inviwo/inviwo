#--------------------------------------------------------------------
# ABufferGL Module
ivw_module(ABufferGL)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES   
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/abuffergeometryglprocessor.h    
	#ABUFFER STUFFS
	${CMAKE_CURRENT_SOURCE_DIR}/abufferglhelpers/abuffergl.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES    
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/abuffergeometryglprocessor.cpp
	#ABUFFER STUFFS
	${CMAKE_CURRENT_SOURCE_DIR}/abufferglhelpers/abuffergl.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES    
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/abuffergeometrygl.frag
    #ABUFFER
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/abuffer/abufferlinkedlist.hglsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/abuffer/abuffersort.hglsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/abuffer/abuffer.hglsl
	${CMAKE_CURRENT_SOURCE_DIR}/glsl/abuffer/abufferresolve.hglsl
	${CMAKE_CURRENT_SOURCE_DIR}/glsl/abuffer/abufferreset.hglsl	
)
ivw_group("Shader Files" ${SHADER_FILES})

#--------------------------------------------------------------------
# Add definition
ivw_add_definition(__CL_ENABLE_EXCEPTIONS)
find_package(OpenMP REQUIRED)
if (OPENMP_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
endif()

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

#--------------------------------------------------------------------
# Package or build shaders into resources
ivw_handle_shader_resources(${CMAKE_CURRENT_SOURCE_DIR}/glsl ${SHADER_FILES})