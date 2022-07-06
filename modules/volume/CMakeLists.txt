ivw_module(Volume)

set(HEADER_FILES
    include/inviwo/volume/algorithm/volumemap.h
    include/inviwo/volume/processors/atlasboundary.h
    include/inviwo/volume/processors/volumeregionmapper.h
    include/inviwo/volume/processors/atlasboundary.h
    include/inviwo/volume/processors/volumeregionstatistics.h
    include/inviwo/volume/processors/volumevoronoisegmentation.h
    include/inviwo/volume/volumemodule.h
    include/inviwo/volume/volumemoduledefine.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/algorithm/volumemap.cpp
    src/processors/atlasboundary.cpp
    src/processors/volumeregionmapper.cpp
    src/processors/atlasboundary.cpp
    src/processors/volumeregionstatistics.cpp
    src/processors/volumevoronoisegmentation.cpp
    src/volumemodule.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

set(SHADER_FILES
	# Add shaders
)
ivw_group("Shader Files" ${SHADER_FILES})

set(TEST_FILES
    tests/unittests/volume-region-map-test.cpp
    tests/unittests/volume-unittest-main.cpp
)
ivw_add_unittest(${TEST_FILES})

ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

# Add shader directory to install package
#ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)

if(IVW_TEST_BENCHMARKS)
    add_subdirectory(tests/benchmarks)
endif()
