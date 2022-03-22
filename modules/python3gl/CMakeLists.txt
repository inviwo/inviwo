ivw_module(Python3GL)

set(HEADER_FILES
    include/inviwo/python3gl/python3glmodule.h
    include/inviwo/python3gl/python3glmoduledefine.h
    include/inviwo/python3gl/volumepytoglconverter.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/python3glmodule.cpp
    src/volumepytoglconverter.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

set(SHADER_FILES
	# Add shaders
)
ivw_group("Shader Files" ${SHADER_FILES})

set(TEST_FILES
    tests/unittests/python3gl-unittest-main.cpp
)
ivw_add_unittest(${TEST_FILES})

ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

# Add shader directory to install package
#ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)
