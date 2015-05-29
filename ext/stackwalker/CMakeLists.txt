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
# Define defintions
ivw_define_standard_definitions(stackwalker)

if(WIN32)
add_definitions( "/W0 /D_CRT_SECURE_NO_WARNINGS /wd4005 /wd4996 /wd4748" )
endif()

#--------------------------------------------------------------------
# Create library

IF(${SHARED_LIBS})
    add_library(stackwalker SHARED ${SOURCE_FILES} ${HEADER_FILES})
ELSE()
    add_library(stackwalker STATIC ${SOURCE_FILES} ${HEADER_FILES})
ENDIF()

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_folder(stackwalker ext)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_set_cpack_name(core)
ivw_make_package(stackwalker stackwalker)