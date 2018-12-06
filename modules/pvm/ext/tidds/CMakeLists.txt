#--------------------------------------------------------------------
# Tiny DDS Package
project(tidds)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/tidds/codebase.h
    include/tidds/ddsbase.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/ddsbase.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Create library
add_library(tidds ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(tidds PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:install>
)

#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(tidds tidds)

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_folder(tidds ext)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_default_install_targets(tidds)
ivw_make_package(TiDDS tidds)
ivw_suppress_compiler_warnings(tidds)
