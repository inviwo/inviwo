ivw_module(OpactOpt)

set(HEADER_FILES
    include/modules/opactopt/opactoptmodule.h
    include/modules/opactopt/opactoptmoduledefine.h
    include/modules/opactopt/processors/opacityoptimiser.h
    include/modules/opactopt/processors/meshmappingvolume.h
    include/modules/opactopt/processors/directopacityoptimisationrenderer.h
    include/modules/opactopt/rendering/opacityoptimisationlinerenderer.h
    include/modules/opactopt/rendering/opacityoptimisationrenderer.h
    include/modules/opactopt/rendering/approximateopacityoptimisationrenderer.h
    include/modules/opactopt/rendering/approximation.h
    include/modules/opactopt/rendering/decoupledopacityoptimisationrenderer.h
    include/modules/opactopt/algorithm/gaussian.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/opactoptmodule.cpp
    src/processors/opacityoptimiser.cpp
    src/processors/meshmappingvolume.cpp
    src/processors/directopacityoptimisationrenderer.cpp
    src/rendering/opacityoptimisationlinerenderer.cpp
    src/rendering/opacityoptimisationrenderer.cpp
    src/rendering/approximateopacityoptimisationrenderer.cpp
    src/rendering/approximation.cpp
    src/rendering/decoupledopacityoptimisationrenderer.cpp
    src/algorithm/gaussian.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

set(SHADER_FILES
    glsl/opactopt/approximate/project.frag
    glsl/opactopt/approximate/smooth.frag
    glsl/opactopt/approximate/blend.frag
    glsl/opactopt/approximate/clear.frag
    glsl/opactopt/approximate/fourier.glsl
    glsl/opactopt/approximate/legendre.glsl
    glsl/opactopt/approximate/piecewise.glsl
    glsl/opactopt/decoupled/display.frag
    glsl/opactopt/direct/clear.frag
    glsl/opactopt/direct/normalise.frag
    glsl/opactopt/direct/background.frag
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

ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

# Add shader directory to install package
ivw_add_to_module_pack(glsl)
