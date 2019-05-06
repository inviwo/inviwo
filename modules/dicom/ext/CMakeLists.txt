#--------------------------------------------------------------------
# Creates the project to configure GDCM on build time

cmake_minimum_required(VERSION 3.0)

project(configure_gdcm)

include(ExternalProject)

ExternalProject_Add(GDCM
SOURCE_DIR ${DICOM_SOURCE_DIR}/${GDCM_EXT_DIR}
BINARY_DIR ${DICOM_BINARY_DIR}/${GDCM_EXT_DIR}
CMAKE_GENERATOR ${DICOM_GENERATOR}
CMAKE_GENERATOR_TOOLSET ${DICOM_GENERATOR_TOOLSET}
CMAKE_GENERATOR_PLATFORM ${DICOM_GENERATOR_PLATFORM}
CMAKE_CACHE_ARGS
    -DGDCM_BUILD_DOCBOOK_MANPAGES:BOOL=OFF
    -DGDCM_ALWAYS_TRACE_MACRO:BOOL=ON

BUILD_COMMAND "" # Disable build step.
INSTALL_COMMAND "" # Disable install step.
)
