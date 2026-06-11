# Tiny DDS Package
project(tidds LANGUAGES CXX C)

set(HEADER_FILES
    include/tidds/codebase.h
    include/tidds/ddsbase.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/ddsbase.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Create library
# needs to be always used as a shared library due to LGPL of Tiny DDS
add_library(tidds)

target_sources(tidds
    PUBLIC
    FILE_SET HEADERS
    TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
        ${HEADER_FILES}
    PRIVATE
        ${SOURCE_FILES}
)
