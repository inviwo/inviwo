ivw_project(libopenjpeg)

#--------------------------------------------------------------------
# Supress warnings
if(WIN32)
add_definitions( "/W0 /D_CRT_SECURE_NO_WARNINGS /wd4005 /wd4996 /nologo /IGNORE:4006" )
endif()

#--------------------------------------------------------------------
# Add header files

file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

ivw_group("Header Files" ${HEADER_FILES} )

#--------------------------------------------------------------------
# Add source files

file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

ivw_group("Source Files" ${SOURCE_FILES} )

#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(libopenjpeg)
if(BUILD_SHARED_LIBS)
    add_definitions(-DOPJ_EXPORTS)
else()
    ivw_add_definition(OPJ_STATIC)
endif()

#--------------------------------------------------------------------
# Create library
add_library(libopenjpeg ${SOURCE_FILES} ${HEADER_FILES})

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_vs_folder(libopenjpeg ext/freeimagelib)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_make_package(LibOpenJPEG libopenjpeg)