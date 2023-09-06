# Following
# http://stackoverflow.com/a/29214327/3214598

set(HEADER_FILES 
    include/flags/allow_flags.h
    include/flags/flags.h
    include/flags/flagsfwd.h
    include/flags/iterator.h
)
ivw_group("Header Files" ${HEADER_FILES})
source_group("CMake Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)

add_library(flags INTERFACE)
add_library(inviwo::flags ALIAS flags)

target_sources(
    flags
    INTERFACE
    FILE_SET HEADERS
    TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${HEADER_FILES}
)

#  HACK: have the files showing in the IDE, under the name 'flags'
add_custom_target(flags_dummy SOURCES ${HEADER_FILES})
ivw_folder(flags_dummy ext)


ivw_register_package(flags flags)
ivw_install_helper(TARGET flags NAMESPACE inviwo DESTINATION inviwo)