project(stackwalker)
set(HEADER_FILES
    include/stackwalker/stackwalker.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/stackwalker.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Create library
add_library(stackwalker ${SOURCE_FILES} ${HEADER_FILES})
add_library(inviwo::stackwalker ALIAS stackwalker)
set_target_properties(stackwalker PROPERTIES VERSION 14.0.0 SOVERSION 14.0)
target_include_directories(stackwalker PUBLIC include PRIVATE include/stackwalker)

ivw_define_standard_definitions(stackwalker stackwalker)

ivw_folder(stackwalker ext)

# Make package (for other projects to find)
ivw_default_install_targets(stackwalker)
ivw_make_package(stackwalker stackwalker)
ivw_suppress_compiler_warnings(stackwalker)
