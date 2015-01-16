ivw_project(ilmimf)

#--------------------------------------------------------------------
# Supress warnings
if(WIN32)
add_definitions( "/W0 /D_CRT_SECURE_NO_WARNINGS /wd4005 /wd4996 /nologo /IGNORE:4006" )
endif()

#--------------------------------------------------------------------
# Add header files

file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

ivw_group("Ilmimf Header Files" ${HEADER_FILES} )

#--------------------------------------------------------------------
# Add source files

file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

ivw_group("Ilmimf Source Files" ${SOURCE_FILES} )

#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(ilmimf)
if(BUILD_SHARED_LIBS)
    ivw_add_definition(OPENEXR_DLL)
endif()

#--------------------------------------------------------------------
# Create library
add_library(ilmimf ${SOURCE_FILES} ${HEADER_FILES})

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_vs_folder(ilmimf ext/freeimagelib/openexrlib)

#--------------------------------------------------------------------
# Add dependencies
ivw_add_dependencies(Half
					Iex
                    Imath
                    InternalZLib)

#ivw_include_directories(${IVW_EXTENSIONS_DIR}/freeimage/openexr/half)
#ivw_include_directories(${IVW_EXTENSIONS_DIR}/freeimage/openexr/imath)
#ivw_include_directories(${IVW_EXTENSIONS_DIR}/freeimage/openexr/ilmimf)
#ivw_include_directories(${IVW_EXTENSIONS_DIR}/freeimage/openexr/ilmthread)
#ivw_include_directories(${IVW_EXTENSIONS_DIR}/freeimage/openexr/iex)
#ivw_include_directories(${IVW_EXTENSIONS_DIR}/freeimage/openexr)
#ivw_include_directories(${IVW_EXTENSIONS_DIR}/freeimage/zlib)
#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_include_directories(${IVW_EXTENSIONS_DIR}/freeimage/openexr/ilmimf)
ivw_make_package(IlmImf ilmimf)