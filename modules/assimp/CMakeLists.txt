#--------------------------------------------------------------------
# Inviwo assimp Module
ivw_module(assimp)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    #${IVW_MODULE_DIR}/assimp/assimpprocessor.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    #${IVW_MODULE_DIR}/assimp/assimpprocessor.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Add shaders
#set(SHADER_FILES
    #${IVW_MODULE_DIR}/assimp/glsl/assimpprocessor.frag
#)
#ivw_group("Shader Files" ${SHADER_FILES})


#--------------------------------------------------------------------
# Add Unittests
#set(TEST_FILES
#	${IVW_MODULE_DIR}/assimp/tests/assimp-test.cpp
#)
#ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

#--------------------------------------------------------------------
# Add shader directory to pack
# ivw_add_to_module_pack(${IVW_MODULE_DIR}/assimp/glsl)