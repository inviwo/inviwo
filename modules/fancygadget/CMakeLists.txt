ivw_module(FancyGadget)

set(HEADER_FILES
    include/inviwo/fancygadget/algorithm/fancymethod.h
    include/inviwo/fancygadget/processors/fancygadgetprocessor.h
    include/inviwo/fancygadget/fancygadgetmodule.h
    include/inviwo/fancygadget/fancygadgetmoduledefine.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/algorithm/fancymethod.cpp
    src/processors/fancygadgetprocessor.cpp
    src/fancygadgetmodule.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

set(SHADER_FILES
    # Add shaders
)
ivw_group("Shader Files" ${SHADER_FILES})

set(TEST_FILES
    tests/unittests/fancygadget-unittest-main.cpp
    tests/unittests/fancymetod-test.cpp
)
ivw_add_unittest(${TEST_FILES})

ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

# Add shader directory to install package
#ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)
