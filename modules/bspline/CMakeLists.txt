#--------------------------------------------------------------------
# Inviwo bspline Module
ivw_module(bspline)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
        include/inviwo/bspline/bsplinemodule.h
        include/inviwo/bspline/bsplinemoduledefine.h
        include/inviwo/bspline/interpolation/nurb.h
        include/inviwo/bspline/interpolation/nurbutilities.h
        include/inviwo/bspline/interpolation/splineinterpolation.h
        ext/tinynurbs/tinynurbs/tinynurbs.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
        src/bsplinemodule.cpp
        src/interpolation/nurb.cpp
        src/interpolation/nurbutilities.cpp
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
    tests/unittests/bspline-unittest-main.cpp
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

#--------------------------------------------------------------------
# Add shader directory to pack
# ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)
