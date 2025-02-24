ivw_module(OpactOpt)

set(HEADER_FILES
    include/modules/opactopt/opactoptmodule.h
    include/modules/opactopt/opactoptmoduledefine.h
    include/modules/opactopt/processors/directopacityoptimisation.h
    include/modules/opactopt/processors/meshmappingvolume.h
    include/modules/opactopt/io/rawpointreader.h
    include/modules/opactopt/utils/approximation.h
    include/modules/opactopt/utils/gaussian.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/opactoptmodule.cpp
    src/processors/directopacityoptimisation.cpp
    src/processors/meshmappingvolume.cpp
    src/io/rawpointreader.cpp
    src/utils/approximation.cpp
    src/utils/gaussian.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

set(SHADER_FILES
    glsl/opactopt/approximate/smooth.frag
    glsl/opactopt/approximate/fourier.glsl
    glsl/opactopt/approximate/legendre.glsl
    glsl/opactopt/approximate/piecewise.glsl
    glsl/opactopt/approximate/powermoments.glsl
    glsl/opactopt/approximate/trigmoments.glsl
    glsl/opactopt/direct/clear.frag
    glsl/opactopt/direct/normalise.frag
    glsl/opactopt/direct/background.frag
    glsl/opactopt/direct/line/linerenderer.vert
    glsl/opactopt/direct/line/projectimportance.frag
    glsl/opactopt/direct/line/approximportancesum.frag
    glsl/opactopt/direct/line/approxblending.frag
    glsl/opactopt/direct/mesh/projectimportance.frag
    glsl/opactopt/direct/mesh/approximportancesum.frag
    glsl/opactopt/direct/mesh/approxblending.frag
    glsl/opactopt/direct/point/projectimportance.frag
    glsl/opactopt/direct/point/approximportancesum.frag
    glsl/opactopt/direct/point/approxblending.frag
)
ivw_group("Shader Files" ${SHADER_FILES})

set(TEST_FILES
    tests/unittests/opactopt-unittest-main.cpp
)
ivw_add_unittest(${TEST_FILES})

ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES} ${SCRIPT_FILES})

# Add shader directory to install package
ivw_add_to_module_pack(glsl)
