project(libpng C)

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
add_library(PNG::PNG ALIAS libpng)
set_target_properties(libpng PROPERTIES VERSION 1.5.13 SOVERSION 1.5)

#--------------------------------------------------------------------
# Define definitions for build on windows
if(BUILD_SHARED_LIBS AND WIN32)
    target_compile_definitions(libpng PRIVATE PNG_BUILD_DLL)
    target_compile_definitions(libpng INTERFACE PNG_USE_DLL)
endif()

#--------------------------------------------------------------------
# Add includes
target_include_directories(libpng INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/ext/libpng>
)

#--------------------------------------------------------------------
# libpng depends on zlib
#find_package(ZLIB REQUIRED) # already done in inviwo/CmakeLists.txt
target_link_libraries(libpng PRIVATE ZLIB::ZLIB)

# Inviwo Settings
ivw_suppress_compiler_warnings(libpng)
ivw_default_install_targets(libpng)
ivw_folder(libpng ext)
source_group("CMake Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)
ivw_make_package(PNG libpng) # Make package (for other projects to find)
