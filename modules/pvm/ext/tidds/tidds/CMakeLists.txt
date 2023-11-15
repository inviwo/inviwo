#--------------------------------------------------------------------
# Tiny DDS Package
project(tidds)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/tidds/codebase.h
    include/tidds/ddsbase.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/ddsbase.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Create library
# needs to be always used as a shared library due to LGPL of Tiny DDS
add_library(tidds ${SOURCE_FILES} ${HEADER_FILES})

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
