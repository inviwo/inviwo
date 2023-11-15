project(stackwalker)

# Create library
add_library(stackwalker)
add_library(inviwo::stackwalker ALIAS stackwalker)
set_target_properties(stackwalker PROPERTIES VERSION 14.0.0 SOVERSION 14.0)
target_include_directories(stackwalker PRIVATE include/stackwalker)

target_sources(stackwalker
    PUBLIC
    FILE_SET HEADERS
    TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        include/stackwalker/stackwalker.h
    PRIVATE
        src/stackwalker.cpp
)

ivw_define_standard_definitions(stackwalker stackwalker)
ivw_suppress_compiler_warnings(stackwalker)
ivw_folder(stackwalker ext)

ivw_make_package(stackwalker stackwalker)
ivw_install_helper(TARGET stackwalker NAMESPACE inviwo DESTINATION inviwo)
