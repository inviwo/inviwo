ivw_project(internallibpng)

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
ivw_define_standard_definitions(internallibpng)
if(BUILD_SHARED_LIBS)
    add_definitions(-DPNG_BUILD_DLL)
    ivw_add_definition_to_list(PNG_USE_DLL)
endif()

#--------------------------------------------------------------------
# Create library
add_library(internallibpng ${SOURCE_FILES} ${HEADER_FILES})


#--------------------------------------------------------------------
# Creates VS folder structure
ivw_vs_folder(internallibpng ext/freeimagelib)

#--------------------------------------------------------------------
# Add dependencies

ivw_add_dependencies(InternalZLib)


INCLUDE_DIRECTORIES("${IVW_EXTENSIONS_DIR}/freeimage/zlib/")

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_make_package(InternalLibPNG internallibpng)


