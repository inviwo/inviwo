ivw_project(libjpeg)

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

## macosx
if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "(Darwin|FreeBSD)")
  list(REMOVE_ITEM SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/jmemmac.c)
endif()

ivw_group("Source Files" ${SOURCE_FILES} )


remove_definitions(-DHAVE_CONFIG_H)
if(BUILD_SHARED_LIBS)
    add_definitions(-DJPEG_DLL)
else()
    ivw_add_definition(JPEG_STATIC)
endif()

#--------------------------------------------------------------------
# Create library
add_library(libjpeg ${SOURCE_FILES} ${HEADER_FILES})

#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(libjpeg libjpeg)

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_folder(libjpeg ext/cimg)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_include_directories(${CMAKE_CURRENT_SOURCE_DIR})
ivw_make_package(LibJPEG libjpeg)