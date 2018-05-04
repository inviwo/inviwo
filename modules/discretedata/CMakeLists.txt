#--------------------------------------------------------------------
# Inviwo discrete data Module
ivw_module(DiscreteData)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
)
#~ ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
)
ivw_group("Sources" ${SOURCE_FILES} ${HEADER_FILES})

#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})

#--------------------------------------------------------------------
# Add shader directory to pack
# ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)
