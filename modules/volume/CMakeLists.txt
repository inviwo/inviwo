ivw_module(Volume)

set(HEADER_FILES
    include/inviwo/volume/processors/concatvolumes.h
    include/inviwo/volume/processors/correlateslices.h
    include/inviwo/volume/processors/volumevoronoisegmentation.h
    include/inviwo/volume/volumemodule.h
    include/inviwo/volume/volumemoduledefine.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/processors/concatvolumes.cpp
    src/processors/correlateslices.cpp
    src/processors/volumevoronoisegmentation.cpp
    src/volumemodule.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

set(SHADER_FILES
	# Add shaders
)
ivw_group("Shader Files" ${SHADER_FILES})

set(TEST_FILES
    tests/unittests/volume-unittest-main.cpp
)
ivw_add_unittest(${TEST_FILES})

ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

# Add shader directory to install package
#ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)
