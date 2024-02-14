ivw_module(Pathtracing)

set(HEADER_FILES
    include/inviwo/pathtracing/pathtracingmodule.h
    include/inviwo/pathtracing/pathtracingmoduledefine.h
    include/inviwo/pathtracing/processors/uniformgridopacity.h
    include/inviwo/pathtracing/processors/volumepathtracer.h
    #include/inviwo/pathtracing/uniformgrid3d.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/pathtracingmodule.cpp
    src/processors/uniformgridopacity.cpp
    src/processors/volumepathtracer.cpp
    # src/uniformgrid3d.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

set(SHADER_FILES
    glsl/bidirectionalvolumepathtracer.comp
    glsl/distributions.glsl
    glsl/lightestimation.glsl
    glsl/random.glsl
    glsl/transmittance/transmittancemethods.glsl
    glsl/uniformgrid.glsl
    glsl/util/rayboxintersection.glsl
    glsl/util/rayminmax.glsl
)
ivw_group("Shader Files" ${SHADER_FILES})

set(TEST_FILES
    tests/unittests/pathtracing-unittest-main.cpp
)
ivw_add_unittest(${TEST_FILES})

ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

# Add shader directory to install package
# ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)
