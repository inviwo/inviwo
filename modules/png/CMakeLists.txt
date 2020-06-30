# Inviwo png Module
ivw_module(png)

set(HEADER_FILES
    include/inviwo/png/pngmodule.h
    include/inviwo/png/pngmoduledefine.h
    include/inviwo/png/pngreader.h
    include/inviwo/png/pngutils.h
    include/inviwo/png/pngwriter.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/pngmodule.cpp
    src/pngreader.cpp
    src/pngutils.cpp
    src/pngwriter.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Add Unittests
set(TEST_FILES
    tests/unittests/png-unittest-main.cpp
    tests/unittests/png-savetobuffer-test.cpp
)
ivw_add_unittest(${TEST_FILES})

# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})

find_package(PNG REQUIRED)
target_link_libraries(inviwo-module-png PRIVATE PNG::PNG)
