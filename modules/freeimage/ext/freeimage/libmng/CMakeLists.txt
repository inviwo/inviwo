#set(BUILD_SHARED_LIBS OFF)

ivw_project(libmng)

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
ivw_define_standard_definitions(libmng)
if(BUILD_SHARED_LIBS)
    add_definitions(-DMNG_BUILD_DLL)
    ivw_add_definition_to_list(MNG_USE_DLL)
endif()

#--------------------------------------------------------------------
# Create library
add_library(libmng ${SOURCE_FILES} ${HEADER_FILES})

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_vs_folder(libmng ext/freeimagelib)

#--------------------------------------------------------------------
# Add dependencies
# ivw_add_dependencies(InternalZLib)

#ivw_add_dependency_libraries(InternalZLib)

INCLUDE_DIRECTORIES("${IVW_EXTENSIONS_DIR}/freeimage/zlib/")
# TARGET_LINK_LIBRARIES(libmng "${IVW_EXTENSIONS_DIR}/freeimage/zlib")
#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_make_package(LibMNG libmng)