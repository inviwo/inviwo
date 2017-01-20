ivw_project(libjpeg C)

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

#--------------------------------------------------------------------
# Create library
add_library(libjpeg ${SOURCE_FILES} ${HEADER_FILES})

#--------------------------------------------------------------------
# Define defintions
if(BUILD_SHARED_LIBS)
	target_compile_definitions(libjpeg PUBLIC JPEG_DLL)
else(BUILD_SHARED_LIBS)
	target_compile_definitions(libjpeg PUBLIC JPEG_STATIC)
endif(BUILD_SHARED_LIBS)

#--------------------------------------------------------------------
# Add includes
target_include_directories(libjpeg INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

#--------------------------------------------------------------------
# Supress warnings
ivw_suppress_compiler_warnings(libjpeg)

#--------------------------------------------------------------------
# Install library into target folder
ivw_default_install_targets(libjpeg)

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_folder(libjpeg ext)
source_group("CMake Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_set_cpack_name(libjpeg)
ivw_make_package(JPEG libjpeg)
