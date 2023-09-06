cmake_minimum_required(VERSION 3.23...3.27 FATAL_ERROR)

set(HEADER_FILES 
    include/half/half.hpp
)
ivw_group("Header Files" ${HEADER_FILES})
source_group("CMake Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)

add_library(half INTERFACE)
add_library(inviwo::half ALIAS half)

target_sources(half
    INTERFACE
    FILE_SET HEADERS
    TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${HEADER_FILES}
)

#  HACK: have the files showing in the IDE, under the name 'half-dummy'
add_custom_target(half_dummy SOURCES ${HEADER_FILES})
ivw_folder(half_dummy ext)

ivw_register_package(half half)
ivw_install_helper(TARGET half NAMESPACE inviwo DESTINATION inviwo)