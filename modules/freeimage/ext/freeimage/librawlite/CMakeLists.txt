ivw_project(librawlite)

#--------------------------------------------------------------------
# Supress warnings
if(WIN32)
add_definitions( "/W0 /D_CRT_SECURE_NO_WARNINGS /wd4005 /wd4996 /nologo /IGNORE:4006" )
endif()

#--------------------------------------------------------------------
# Add header files

file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
list(REMOVE_ITEM HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/internal/libraw_internal_funcs.h)
ivw_group("Header Files" ${HEADER_FILES} )

#--------------------------------------------------------------------
# Add source files

file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

ivw_group("Source Files" ${SOURCE_FILES} )

#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(librawlite)
if(BUILD_SHARED_LIBS)
   add_definitions(-DLIBRAW_BUILDLIB)
else()
   ivw_add_definition(LIBRAW_NODLL)
endif()
add_definitions( -DNODEPS)

#--------------------------------------------------------------------
# Create library
add_library(librawlite ${SOURCE_FILES} ${HEADER_FILES})

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_folder(librawlite ext/freeimagelib)

#--------------------------------------------------------------------
# Add dependencies
ivw_add_dependencies(InternalZLib)

#--------------------------------------------------------------------
# Link folders
ivw_include_directories(${IVW_EXTENSIONS_DIR}/freeimage/librawlite)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_make_package(LibRawLite librawlite)