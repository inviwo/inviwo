#--------------------------------------------------------------------
# Inviwo bspline Module
ivw_module(bspline)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
        include/inviwo/bspline/bsplinemodule.h
        include/inviwo/bspline/bsplinemoduledefine.h
        include/inviwo/bspline/interpolation/nurbutilities.h
        include/inviwo/bspline/interpolation/splineinterpolation.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
        src/bsplinemodule.cpp
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
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

#target_link_libraries(inviwo-module-bspline PRIVATE inviwo::tinynurbs)
#
#ivw_register_license_file(ID tinynurbs NAME "Tinynurbs" TARGET inviwo::tinynurbs MODULE bspline
#        URL https://github.com/pradeep-pyro/tinynurbs
#        FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/tinynurbs/LICENSE
#        )
#

#--------------------------------------------------------------------
# Add shader directory to pack
# ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)
