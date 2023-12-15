ivw_module(Pathtracing)

set(HEADER_FILES
    include/inviwo/pathtracing/pathtracingmodule.h
    include/inviwo/pathtracing/pathtracingmoduledefine.h
    include/inviwo/pathtracing/processors/volumepathtracer.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/pathtracingmodule.cpp
    src/processors/volumepathtracer.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

set(SHADER_FILES
    glsl/util/vectormatrixmethods.glsl
    glsl/random.glsl
    glsl/transmittance/transmittancemethods.glsl
    glsl/lightestimation.glsl
    glsl/bidirectionalvolumepathtracer.comp
)
ivw_group("Shader Files" ${SHADER_FILES})

set(TEST_FILES
    tests/unittests/pathtracing-unittest-main.cpp
)
ivw_add_unittest(${TEST_FILES})

ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

# Add shader directory to install package
# ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)
