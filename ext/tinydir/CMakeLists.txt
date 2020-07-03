set(HEADER_FILES tinydir.h)
ivw_group("Header Files" ${HEADER_FILES})
source_group("CMake Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)

add_library(tinydir INTERFACE) # 'moduleA' is an INTERFACE pseudo target
add_library(inviwo::tinydir ALIAS tinydir)
target_include_directories(tinydir INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

#  HACK: have the files showing in the IDE, under the name 'tinydir_dummy'
add_custom_target(tinydir_dummy SOURCES ${HEADER_FILES})
ivw_folder(tinydir_dummy ext)

ivw_default_install_targets(tinydir)
ivw_make_package(tinydir tinydir)
