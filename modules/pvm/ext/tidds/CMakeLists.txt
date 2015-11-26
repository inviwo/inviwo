#--------------------------------------------------------------------
# Tiny DDS Package
ivw_project(tidds)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/codebase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ddsbase.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/ddsbase.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Create library
add_library(tidds ${SOURCE_FILES} ${HEADER_FILES})

#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(tidds tidds)

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_folder(tidds ext)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_make_package(TiDDS tidds)