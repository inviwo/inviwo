#--------------------------------------------------------------------
# Inviwo json Module
ivw_module(json)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/inviwo/json/io/jsonreader.h
    include/inviwo/json/jsonmodule.h
    include/inviwo/json/jsonmoduledefine.h
    include/inviwo/json/jsonutils.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/io/jsonreader.cpp
    src/jsonmodule.cpp
    src/jsonutils.cpp
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
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})
target_link_libraries(inviwo-module-json PUBLIC
	nlohmann_json::nlohmann_json
)

#--------------------------------------------------------------------
# Add shader directory to pack
# ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)
