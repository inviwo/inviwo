project(libpng C)

# Add header files
file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
ivw_group("Header Files" ${HEADER_FILES} )

# Add source files
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
ivw_group("Source Files" ${SOURCE_FILES} )

# Create library
add_library(png ${SOURCE_FILES} ${HEADER_FILES})
add_library(PNG::PNG ALIAS png)
set_target_properties(png PROPERTIES VERSION 1.5.13 SOVERSION 1.5)


# Define definitions for build on windows
if(BUILD_SHARED_LIBS AND WIN32)
    target_compile_definitions(png PRIVATE PNG_BUILD_DLL)
    target_compile_definitions(png INTERFACE PNG_USE_DLL)
endif()

# Add includes
target_include_directories(png INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/ext/png>
)

# png depends on zlib
find_package(ZLIB CONFIG REQUIRED)
target_link_libraries(png PRIVATE ZLIB::ZLIB)

# Inviwo Settings
ivw_suppress_compiler_warnings(png)
ivw_default_install_targets(png)
ivw_folder(png ext)
source_group("CMake Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)
ivw_make_package(PNG png) # Make package (for other projects to find)
