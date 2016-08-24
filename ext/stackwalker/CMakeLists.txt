#--------------------------------------------------------------------
# stackwalker Package
ivw_project(stackwalker)
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

IF(${SHARED_LIBS})
    add_library(stackwalker SHARED ${SOURCE_FILES} ${HEADER_FILES})
ELSE()
    add_library(stackwalker STATIC ${SOURCE_FILES} ${HEADER_FILES})
ENDIF()

#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(stackwalker stackwalker)

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_folder(stackwalker ext)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_set_cpack_name(core)
ivw_make_package(stackwalker stackwalker)
ivw_suppress_compiler_warnings(stackwalker)