# Following
# http://stackoverflow.com/a/29214327/3214598

file(GLOB_RECURSE HEADER_FILES include/tclap/*.h)

ivw_group("Header Files" BASE include/tclap ${HEADER_FILES})
source_group("CMake Files" FILES CMakeLists.txt)

add_library(tclap INTERFACE)
add_library(inviwo::tclap ALIAS tclap)

target_sources(tclap
    INTERFACE
    FILE_SET HEADERS
    TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${HEADER_FILES}
)

#  HACK: have the files showing in the IDE, under the name 'flags'
add_custom_target(tclap_dummy SOURCES ${HEADER_FILES})
set_target_properties(tclap_dummy PROPERTIES VERSION 1.2.5 SOVERSION 1.2)

ivw_register_package(tclap tclap)
# Creates VS folder structure
ivw_folder(tclap_dummy ext)

ivw_install_helper(TARGET tclap NAMESPACE inviwo DESTINATION inviwo)