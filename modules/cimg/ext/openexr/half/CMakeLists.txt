ivw_project(half)

#--------------------------------------------------------------------
# Supress warnings
if(WIN32)
add_definitions( "/W0 /D_CRT_SECURE_NO_WARNINGS /wd4005 /wd4996 /nologo /IGNORE:4006" )
endif()

#--------------------------------------------------------------------
# Add header files

file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

ivw_group("Half Header Files" ${HEADER_FILES} )

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    eLut.cpp
    half.cpp
    toFloat.cpp
)
ivw_group("Half Source Files" ${SOURCE_FILES} )

#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(half)
if(BUILD_SHARED_LIBS)
    ivw_add_definition(OPENEXR_DLL)
endif()

#--------------------------------------------------------------------
# Create library
add_library(half ${SOURCE_FILES} ${HEADER_FILES})

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_folder(half ext/freeimagelib/openexrlib)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_include_directories(${IVW_EXTENSIONS_DIR}/freeimage/openexr/half)
ivw_make_package(Half half)