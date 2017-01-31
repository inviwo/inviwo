ivw_project(libpng C)

#--------------------------------------------------------------------
# Add header files
file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

ivw_group("Header Files" ${HEADER_FILES} )

#--------------------------------------------------------------------
# Add source files
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

ivw_group("Source Files" ${SOURCE_FILES} )

#--------------------------------------------------------------------
# Create library
add_library(libpng ${SOURCE_FILES} ${HEADER_FILES})

#--------------------------------------------------------------------
# Define definitions for build on windows
if(BUILD_SHARED_LIBS AND WIN32)
    target_compile_definitions(libpng PRIVATE PNG_BUILD_DLL)
    ivw_add_definition_to_list(PNG_USE_DLL)
endif()

#--------------------------------------------------------------------
# Add includes
target_include_directories(libpng INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

#--------------------------------------------------------------------
# libpng depends on zlib
find_package(ZLIB REQUIRED)
target_include_directories(libpng PRIVATE ${ZLIB_INCLUDE_DIR})
target_link_libraries(libpng PRIVATE ${ZLIB_LIBRARIES})
add_dependencies(libpng zlib1)

#--------------------------------------------------------------------
# Supress warnings
ivw_suppress_compiler_warnings(libpng)

#--------------------------------------------------------------------
# Install library into target folder
ivw_default_install_targets(libpng)

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_folder(libpng ext)

source_group("CMake Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_set_cpack_name(libpng)
ivw_make_package(PNG libpng)
