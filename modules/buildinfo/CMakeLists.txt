#--------------------------------------------------------------------
# Inviwo BuildInfo Module
ivw_module(BuildInfo)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    #${CMAKE_CURRENT_SOURCE_DIR}/buildinfoprocessor.h
    ${CMAKE_BINARY_DIR}/modules/_generated/inviwo_buildinfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/buildinfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/buildinfomodule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/buildinfomoduledefine.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/buildinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/buildinfomodule.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
	#${CMAKE_CURRENT_SOURCE_DIR}/tests/buildinfo-test.cpp
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Disable Pre-compiled headers
ivw_set_pch_disabled_for_module()

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})

#--------------------------------------------------------------------
# Custom target for updating the build information (this target is always out-of-date)
add_custom_target(buildinfo-header ALL
    BYPRODUCTS "${CMAKE_BINARY_DIR}/modules/_generated/inviwo_buildinfo.h"
    COMMAND ${CMAKE_COMMAND} -P "${IVW_CMAKE_BINARY_MODULE_DIR}/build_info.cmake"
    #COMMAND ${CMAKE_COMMAND} -E touch "${CMAKE_BINARY_DIR}/modules/_generated/inviwo_buildinfo.stamp"
    WORKING_DIRECTORY ${OUTPUT_DIR}
    COMMENT "Generating inviwo build info (buildinfo)"
    VERBATIM
)
add_dependencies("inviwo-module-buildinfo" "buildinfo-header")
