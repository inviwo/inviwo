#--------------------------------------------------------------------
# Inviwo animationsplineinterpolation Module
ivw_module(animationsplineinterpolation)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/inviwo/animationsplineinterpolation/animationsplineinterpolationmodule.h
    include/inviwo/animationsplineinterpolation/animationsplineinterpolationmoduledefine.h
        include/inviwo/animationsplineinterpolation/interpolation/nurbsinterpolation.h
        include/inviwo/animationsplineinterpolation/interpolation/splineinterpolation.h
        include/inviwo/animationsplineinterpolation/interpolation/nurb.h
        )
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/animationsplineinterpolationmodule.cpp
        src/interpolation/nurbsinterpolation.cpp
        src/interpolation/nurb.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
)
ivw_group("Shader Files" ${SHADER_FILES})

#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
    tests/unittests/animationsplineinterpolation-unittest-main.cpp
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

#--------------------------------------------------------------------
# Add shader directory to pack
# ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)
