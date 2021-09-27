#--------------------------------------------------------------------
# Inviwo ContourTree Module
ivw_module(ContourTree)

#--------------------------------------------------------------------
# Build libContourTree
add_subdirectory(libContourTree)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/modules/contourtree/contourtreemodule.h
    include/modules/contourtree/contourtreemoduledefine.h
    include/modules/contourtree/processors/contourtree.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/contourtreemodule.cpp
    src/processors/contourtree.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
)
ivw_group("Shader Files" ${SHADER_FILES})

#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
    tests/unittests/contourtree-unittest-main.cpp
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

target_include_directories(inviwo-module-contourtree PUBLIC 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/libContourTree/include>
)

#--------------------------------------------------------------------
# Add shader directory to pack
# ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)
