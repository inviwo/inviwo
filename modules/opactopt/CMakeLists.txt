ivw_module(OpactOpt)

set(HEADER_FILES
    include/modules/opactopt/opactoptmodule.h
    include/modules/opactopt/opactoptmoduledefine.h
    include/modules/opactopt/processors/directopacityoptimization.h
    include/modules/opactopt/processors/meshmappingvolume.h
    include/modules/opactopt/utils/approximation.h
    include/modules/opactopt/utils/gaussian.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/opactoptmodule.cpp
    src/processors/directopacityoptimization.cpp
    src/processors/meshmappingvolume.cpp
    src/utils/approximation.cpp
    src/utils/gaussian.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

set(SHADER_FILES
    glsl/opactopt/common.glsl
    glsl/opactopt/debug.glsl
    glsl/opactopt/importance.glsl
    glsl/opactopt/clear.frag
    glsl/opactopt/normalise.frag
    glsl/opactopt/smooth.frag
    glsl/opactopt/approximation/fourier.glsl
    glsl/opactopt/approximation/legendre.glsl
    glsl/opactopt/approximation/piecewise.glsl
    glsl/opactopt/approximation/powermoments.glsl
    glsl/opactopt/approximation/trigmoments.glsl
    glsl/opactopt/approximation/powermomentmaths.glsl
    glsl/opactopt/approximation/trigmomentmaths.glsl
    glsl/opactopt/line/linerenderer.vert
    glsl/opactopt/line/projectimportance.frag
    glsl/opactopt/line/approximportancesum.frag
    glsl/opactopt/line/approxblending.frag
    glsl/opactopt/mesh/projectimportance.frag
    glsl/opactopt/mesh/approximportancesum.frag
    glsl/opactopt/mesh/approxblending.frag
    glsl/opactopt/point/basicpointrenderer.vert
    glsl/opactopt/point/projectimportance.frag
    glsl/opactopt/point/approximportancesum.frag
    glsl/opactopt/point/approxblending.frag
)
ivw_group("Shader Files" ${SHADER_FILES})

set(TEST_FILES
    tests/unittests/opactopt-unittest-main.cpp
)
ivw_add_unittest(${TEST_FILES})

ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES} ${SCRIPT_FILES})

# Package or build shaders into resources
ivw_handle_shader_resources(${CMAKE_CURRENT_SOURCE_DIR}/glsl ${SHADER_FILES})
