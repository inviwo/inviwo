#--------------------------------------------------------------------
# stackwalker Package
if(NOT WIN32)
	return()
endif()

project(stackwalker)
#--------------------------------------------------------------------

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${IVW_EXTENSIONS_DIR}/stackwalker/stackwalker.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${IVW_EXTENSIONS_DIR}/stackwalker/stackwalker.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Create library
add_library(stackwalker ${SOURCE_FILES} ${HEADER_FILES})
add_library(inviwo::stackwalker ALIAS stackwalker)
set_target_properties(stackwalker PROPERTIES VERSION 14.0.0 SOVERSION 14.0)

#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(stackwalker stackwalker)

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_folder(stackwalker ext)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_default_install_comp_targets(core stackwalker)
ivw_make_package(stackwalker stackwalker)
ivw_suppress_compiler_warnings(stackwalker)