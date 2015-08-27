ivw_project(zlib)

#--------------------------------------------------------------------
# Supress warnings
if(WIN32)

add_definitions( "/W0 /D_CRT_SECURE_NO_WARNINGS /wd4005 /wd4996 /nologo /IGNORE:4006" )

if(NOT DESIRED_QT_VERSION MATCHES 5)    
    # Certain zlib functions conflict with Qt4 or less. Hence Z_PREFIX is requried.
    add_definitions( "/DZ_PREFIX" )
endif()

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
ivw_define_standard_definitions(zlib)
if(BUILD_SHARED_LIBS)
    add_definitions(-DZLIB_INTERNAL)
    ivw_add_definition(ZLIB_DLL)
endif()

#--------------------------------------------------------------------
# Create library
add_library(zlib ${SOURCE_FILES} ${HEADER_FILES})

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_folder(zlib ext/cimg)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_include_directories(${CMAKE_CURRENT_SOURCE_DIR})
ivw_make_package(InternalZLib zlib)