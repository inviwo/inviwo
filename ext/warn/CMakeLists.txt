file(GLOB_RECURSE ignore_files include/warn/ignore/*)
set(HEADER_FILES 
    ${ignore_files}
    include/warn/pop
    include/warn/push
)
set(TEXT_FILES 
    include/warn/warnings.md
)

ivw_group("Header Files" BASE include/warn ${HEADER_FILES})
ivw_group("Text Files" BASE include/warn ${TEXT_FILES})
source_group("CMake Files" FILES CMakeLists.txt)

add_library(warn INTERFACE)
add_library(inviwo::warn ALIAS warn)
target_include_directories(warn INTERFACE 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
) 

add_custom_target(warn_dummy SOURCES ${HEADER_FILES} ${TEXT_FILES})
ivw_folder(warn_dummy ext)

ivw_register_package(warn warn)
ivw_install_helper(TARGET warn NAMESPACE inviwo DESTINATION inviwo)
