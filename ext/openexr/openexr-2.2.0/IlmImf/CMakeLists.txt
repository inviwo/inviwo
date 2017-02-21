# yue.nicholas@gmail.com

SET(CMAKE_INCLUDE_CURRENT_DIR 1)

IF(${OpenEXR_GENERATE_LUT})
  ADD_EXECUTABLE ( b44ExpLogTable
    b44ExpLogTable.cpp
  )

  TARGET_LINK_LIBRARIES ( b44ExpLogTable
    openexr-half
    openexr-iex
    openexr-ilmthread
    # Half
    # Iex${ILMBASE_LIBSUFFIX}
    # IlmThread${ILMBASE_LIBSUFFIX}
    ${PTHREAD_LIB}
  )

  ADD_CUSTOM_COMMAND (
    OUTPUT ${CMAKE_BINARY_DIR}/modules/_generated/ext/openexr/b44ExpLogTable.h
    COMMAND b44ExpLogTable > ${CMAKE_BINARY_DIR}/modules/_generated/ext/openexr/b44ExpLogTable.h
    DEPENDS b44ExpLogTable
  )

  ADD_EXECUTABLE ( dwaLookups
    dwaLookups.cpp
  )

  TARGET_LINK_LIBRARIES ( dwaLookups
    openexr-half
    openexr-iex
    openexr-ilmthread
    # Half
    # Iex${ILMBASE_LIBSUFFIX}
    # IlmThread${ILMBASE_LIBSUFFIX}
    ${PTHREAD_LIB}
  )

  ADD_CUSTOM_COMMAND (
    OUTPUT ${CMAKE_BINARY_DIR}/modules/_generated/ext/openexr/dwaLookups.h
    COMMAND dwaLookups > ${CMAKE_BINARY_DIR}/modules/_generated/ext/openexr/dwaLookups.h
    DEPENDS dwaLookups
  )
ENDIF()

SET ( ILMIMF_SRCS
  ImfAttribute.cpp
  ImfBoxAttribute.cpp
  ImfCRgbaFile.cpp
  ImfChannelList.cpp
  ImfChannelListAttribute.cpp
  ImfFloatAttribute.cpp
  ImfFrameBuffer.cpp
  ImfHeader.cpp
  ImfIO.cpp
  ImfInputFile.cpp
  ImfIntAttribute.cpp
  ImfLineOrderAttribute.cpp
  ImfMatrixAttribute.cpp
  ImfOpaqueAttribute.cpp
  ImfOutputFile.cpp
  ImfRgbaFile.cpp
  ImfStringAttribute.cpp
  ImfVecAttribute.cpp
  ImfHuf.cpp
  ImfThreading.cpp
  ImfWav.cpp
  ImfLut.cpp
  ImfCompressor.cpp
  ImfRleCompressor.cpp
  ImfZipCompressor.cpp
  ImfPizCompressor.cpp
  ImfB44Compressor.cpp
  ImfDwaCompressor.cpp
  ImfMisc.cpp
  ImfCompressionAttribute.cpp
  ImfDoubleAttribute.cpp
  ImfConvert.cpp
  ImfPreviewImage.cpp
  ImfPreviewImageAttribute.cpp
  ImfVersion.cpp
  ImfChromaticities.cpp
  ImfChromaticitiesAttribute.cpp
  ImfKeyCode.cpp
  ImfKeyCodeAttribute.cpp
  ImfTimeCode.cpp
  ImfTimeCodeAttribute.cpp
  ImfRational.cpp
  ImfRationalAttribute.cpp
  ImfFramesPerSecond.cpp
  ImfStandardAttributes.cpp
  ImfStdIO.cpp
  ImfEnvmap.cpp
  ImfEnvmapAttribute.cpp
  ImfScanLineInputFile.cpp
  ImfTiledInputFile.cpp
  ImfTiledMisc.cpp
  ImfTiledOutputFile.cpp
  ImfTiledRgbaFile.cpp
  ImfTileDescriptionAttribute.cpp
  ImfTileOffsets.cpp
  ImfRgbaYca.cpp
  ImfPxr24Compressor.cpp
  ImfTestFile.cpp
  ImfStringVectorAttribute.cpp
  ImfMultiView.cpp
  ImfAcesFile.cpp
  ImfMultiPartOutputFile.cpp
  ImfGenericOutputFile.cpp
  ImfOutputPartData.cpp
  ImfMultiPartInputFile.cpp
  ImfGenericInputFile.cpp
  ImfPartType.cpp
  ImfInputPartData.cpp
  ImfOutputPart.cpp
  ImfTiledOutputPart.cpp
  ImfInputPart.cpp
  ImfTiledInputPart.cpp
  ImfDeepScanLineInputPart.cpp
  ImfDeepScanLineOutputPart.cpp
  ImfDeepScanLineInputFile.cpp
  ImfDeepScanLineOutputFile.cpp
  ImfDeepTiledInputPart.cpp
  ImfDeepTiledOutputPart.cpp
  ImfDeepTiledInputFile.cpp
  ImfDeepTiledOutputFile.cpp
  ImfDeepFrameBuffer.cpp
  ImfDeepCompositing.cpp
  ImfCompositeDeepScanLine.cpp
  ImfDeepImageStateAttribute.cpp
  ImfFastHuf.cpp
  ImfFloatVectorAttribute.cpp
  ImfRle.cpp
  ImfSystemSpecific.cpp
  ImfZip.cpp
)

IF (BUILD_SHARED_LIBS)
    ADD_DEFINITIONS(-DILMIMF_EXPORTS)
ENDIF()

ADD_LIBRARY ( openexr-ilmimf ${LIB_TYPE}
  ${ILMIMF_SRCS}
)

TARGET_LINK_LIBRARIES ( openexr-ilmimf
  openexr-half
  openexr-iex
  openexr-imath
  openexr-ilmthread
  # Half 
  # Iex${ILMBASE_LIBSUFFIX}
  # Imath${ILMBASE_LIBSUFFIX}
  # IlmThread${ILMBASE_LIBSUFFIX}
  ${PTHREAD_LIB} ${ZLIB_LIBRARIES}
)

IF(${OpenEXR_GENERATE_LUT})
  ADD_DEPENDENCIES ( openexr-ilmimf b44ExpLogTable )
ELSE()
  INCLUDE_DIRECTORIES(${lookup_table_dir})
ENDIF()

SET_SOURCE_FILES_PROPERTIES (
  ImfB44Compressor.cpp
  PROPERTIES
  OBJECT_DEPENDS ${lookup_table_dir}/b44ExpLogTable.h
)

IF(${OpenEXR_GENERATE_LUT})
  ADD_DEPENDENCIES ( openexr-ilmimf dwaLookups )
ELSE()
  INCLUDE_DIRECTORIES(${lookup_table_dir})
ENDIF()

SET_SOURCE_FILES_PROPERTIES (
  ImfDwaCompressor.cpp
  PROPERTIES
  OBJECT_DEPENDS ${lookup_table_dir}/dwaLookups.h
)

# Libraries

# INSTALL ( TARGETS
#   IlmImf
#   DESTINATION
#   ${CMAKE_INSTALL_PREFIX}/lib
# )

# # Headers
# INSTALL ( FILES
#   ImfForward.h
#   ImfExport.h
#   ImfAttribute.h
#   ImfBoxAttribute.h
#   ImfCRgbaFile.h
#   ImfChannelList.h
#   ImfChannelListAttribute.h
#   ImfCompressionAttribute.h
#   ImfDoubleAttribute.h
#   ImfFloatAttribute.h
#   ImfFrameBuffer.h
#   ImfHeader.h
#   ImfIO.h
#   ImfInputFile.h
#   ImfIntAttribute.h
#   ImfLineOrderAttribute.h
#   ImfMatrixAttribute.h
#   ImfOpaqueAttribute.h
#   ImfOutputFile.h
#   ImfRgbaFile.h
#   ImfStringAttribute.h
#   ImfVecAttribute.h
#   ImfHuf.h
#   ImfWav.h
#   ImfLut.h
#   ImfArray.h
#   ImfCompression.h
#   ImfLineOrder.h
#   ImfName.h
#   ImfPixelType.h
#   ImfVersion.h
#   ImfXdr.h
#   ImfConvert.h
#   ImfPreviewImage.h
#   ImfPreviewImageAttribute.h
#   ImfChromaticities.h
#   ImfChromaticitiesAttribute.h
#   ImfKeyCode.h
#   ImfKeyCodeAttribute.h
#   ImfTimeCode.h
#   ImfTimeCodeAttribute.h
#   ImfRational.h
#   ImfRationalAttribute.h
#   ImfFramesPerSecond.h
#   ImfStandardAttributes.h
#   ImfEnvmap.h
#   ImfEnvmapAttribute.h
#   ImfInt64.h
#   ImfRgba.h
#   ImfTileDescription.h
#   ImfTileDescriptionAttribute.h
#   ImfTiledInputFile.h
#   ImfTiledOutputFile.h
#   ImfTiledRgbaFile.h
#   ImfRgbaYca.h
#   ImfTestFile.h
#   ImfThreading.h
#   ImfB44Compressor.h
#   ImfStringVectorAttribute.h
#   ImfMultiView.h
#   ImfAcesFile.h
#   ImfMultiPartOutputFile.h
#   ImfGenericOutputFile.h
#   ImfMultiPartInputFile.h
#   ImfGenericInputFile.h
#   ImfPartType.h
#   ImfPartHelper.h
#   ImfOutputPart.h
#   ImfTiledOutputPart.h
#   ImfInputPart.h
#   ImfTiledInputPart.h
#   ImfDeepScanLineOutputFile.h
#   ImfDeepScanLineOutputPart.h
#   ImfDeepScanLineInputFile.h
#   ImfDeepScanLineInputPart.h
#   ImfDeepTiledInputFile.h
#   ImfDeepTiledInputPart.h
#   ImfDeepTiledOutputFile.h
#   ImfDeepTiledOutputPart.h
#   ImfDeepFrameBuffer.h
#   ImfDeepCompositing.h
#   ImfCompositeDeepScanLine.h
#   ImfNamespace.h
#   ImfMisc.h
#   ImfDeepImageState.h
#   ImfDeepImageStateAttribute.h
#   ImfFloatVectorAttribute.h
#   DESTINATION
#   ${CMAKE_INSTALL_PREFIX}/include/OpenEXR
# )


#--------------------------------------------------------------------
# Supress warnings
ivw_suppress_compiler_warnings(openexr-ilmimf)

#--------------------------------------------------------------------
# Install library into target folder
ivw_default_install_targets(openexr-ilmimf)

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_folder(openexr-ilmimf ext/OpenEXR)
IF(${OpenEXR_GENERATE_LUT})
  ivw_folder(b44ExpLogTable ext/OpenEXR/util)
  ivw_folder(dwaLookups ext/OpenEXR/util)
ENDIF()
source_group("CMake Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_set_cpack_name(openexr-ilmimf)
ivw_make_package(openexr-ilmimf openexr-ilmimf)
