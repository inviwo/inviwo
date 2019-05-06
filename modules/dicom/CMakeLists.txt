#--------------------------------------------------------------------
# Inviwo DICOM Module
ivw_module(DICOM)

#--------------------------------------------------------------------
# Include the GDCM Library

option(IVW_MODULE_DICOM_USE_SYSTEM_GDCM "Use system gdcm library" FALSE)
mark_as_advanced(IVW_MODULE_DICOM_USE_SYSTEM_GDCM)

if(NOT IVW_MODULE_DICOM_USE_SYSTEM_GDCM)

include(ExternalProject)
set(GDCM_EXT_DIR ext/gdcm)

# Create the gdcm directory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${GDCM_EXT_DIR})


# Create a project for the configure step using ext/CMakeLists.txt
execute_process(
    COMMAND ${CMAKE_COMMAND}
    ${CMAKE_CURRENT_SOURCE_DIR}/ext
    -DDICOM_SOURCE_DIR:PATH=${CMAKE_CURRENT_SOURCE_DIR}
    -DDICOM_BINARY_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR}
    -DGDCM_EXT_DIR:PATH=${GDCM_EXT_DIR}
    -DDICOM_GENERATOR:STRING=${CMAKE_GENERATOR}
    -DDICOM_GENERATOR_PLATFORM:STRING=${CMAKE_GENERATOR_PLATFORM}
    -DDICOM_GENERATOR_TOOLSET:STRING=${CMAKE_GENERATOR_TOOLSET}
    -G ${CMAKE_GENERATOR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/ext
)

# Build the project for the configure step
execute_process(
    COMMAND ${CMAKE_COMMAND}
    --build .
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/ext
)

# Add the already configured GDCM to the Inviwo build
ExternalProject_Add(GDCM
SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${GDCM_EXT_DIR}
BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/${GDCM_EXT_DIR}
CMAKE_GENERATOR ${CMAKE_GENERATOR}
CMAKE_GENERATOR_TOOLSET ${CMAKE_GENERATOR_TOOLSET}
CMAKE_GENERATOR_PLATFORM ${CMAKE_GENERATOR_PLATFORM}
CMAKE_CACHE_ARGS
    -DGDCM_BUILD_DOCBOOK_MANPAGES:BOOL=OFF
    -DGDCM_ALWAYS_TRACE_MACRO:BOOL=ON

#CONFIGURE_COMMAND ""            # configure step should not be needed, but if removed it fails to build in visual studio
INSTALL_COMMAND ""              # remove install command as we use it in the build path
UPDATE_COMMAND ""
)

# Set tje GDCM_DIR to the previously configured directory and add the package
set(GDCM_DIR ${CMAKE_CURRENT_BINARY_DIR}/${GDCM_EXT_DIR})
endif()

find_package(GDCM REQUIRED)
include(${GDCM_USE_FILE})


#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/modules/dicom/io/gdcmvolumereader.h
    include/modules/dicom/io/mevisvolumereader.h
    include/modules/dicom/dicommodule.h
    include/modules/dicom/dicommoduledefine.h
    include/modules/dicom/errorlogging.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/io/gdcmvolumereader.cpp
    src/io/mevisvolumereader.cpp
    src/dicommodule.cpp
    src/errorlogging.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(inviwo-module-dicom PUBLIC
    ${GDCM_INCLUDE_DIRS}
)

# we need the libtiff4 sources, so add it as dependency
# but actually not freeimage functionality is used
# TODO: link instead of compile with the libtiff sources
# => modification of the freeimage library/module needed!

list(APPEND GDCM_TARGETS gdcmCommon gdcmDICT gdcmDSED gdcmIOD gdcmMSFF gdcmMEXD gdcmcharls gdcmjpeg8 gdcmjpeg12 gdcmjpeg16 gdcmexpat gdcmopenjp2 gdcmzlib socketxx)
target_link_libraries(inviwo-module-dicom PUBLIC inviwo::tiff ${GDCM_TARGETS})
