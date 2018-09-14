# Add grassroots Dicom reader
set(GDCM_ALWAYS_TRACE_MACRO ON CACHE BOOL "enable errors/warnings/debug messages from GDCM" FORCE)

add_subdirectory(ext/gdcm-2.4.4)
ivw_folder(gdcmCommon ext/gdcm/)
ivw_folder(gdcmDICT ext/gdcm/)
ivw_folder(gdcmDSED ext/gdcm/)
ivw_folder(gdcmIOD ext/gdcm/)
ivw_folder(gdcmMSFF ext/gdcm/)
ivw_folder(gdcmMEXD ext/gdcm/)
ivw_folder(gdcmcharls ext/gdcm/)
ivw_folder(gdcmzlib ext/gdcm/)
ivw_folder(gdcmjpeg8 ext/gdcm/)
ivw_folder(gdcmjpeg12 ext/gdcm/)
ivw_folder(gdcmjpeg16 ext/gdcm/)
ivw_folder(gdcmexpat ext/gdcm/)
ivw_folder(gdcmopenjpeg ext/gdcm/)
ivw_folder(socketxx ext/gdcm/)

#--------------------------------------------------------------------
# Inviwo DICOM Module
ivw_module(DICOM)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/io/gdcmvolumereader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/io/mevisvolumereader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/errorlogging.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/io/gdcmvolumereader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/io/mevisvolumereader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/errorlogging.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(inviwo-module-dicom PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/ext/gdcm-2.4.4/Source/
   ${CMAKE_CURRENT_SOURCE_DIR}/ext/gdcm-2.4.4/Source/DataStructureAndEncodingDefinition
   ${CMAKE_CURRENT_SOURCE_DIR}/ext/gdcm-2.4.4/Source/Common
   ${CMAKE_CURRENT_SOURCE_DIR}/ext/gdcm-2.4.4/Source/Attribute
   ${CMAKE_CURRENT_SOURCE_DIR}/ext/gdcm-2.4.4/Source/DataDictionary
   ${CMAKE_CURRENT_SOURCE_DIR}/ext/gdcm-2.4.4/Source/InformationObjectDefinition
   ${CMAKE_CURRENT_SOURCE_DIR}/ext/gdcm-2.4.4/Source/MessageExchangeDefinition
   # gdcmConfigure.h
   ${GDCM_BINARY_DIR}/Source/Common/
)

# we need the libtiff4 sources, so add it as dependency
# but actually not freeimage functionality is used
# TODO: link instead of compile with the libtiff sources
# => modification of the freeimage library/module needed!
target_link_libraries(inviwo-module-dicom PUBLIC inviwo::tiff gdcmCommon gdcmDICT gdcmDSED gdcmIOD gdcmMSFF gdcmMEXD gdcmcharls gdcmzlib gdcmjpeg8 gdcmjpeg12 gdcmjpeg16 gdcmexpat gdcmopenjpeg)
