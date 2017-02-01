
SET ( ILMIMFUTIL_SRCS
  ImfImageChannel.cpp
  ImfFlatImageChannel.cpp
  ImfDeepImageChannel.cpp
  ImfSampleCountChannel.cpp
  ImfImageLevel.cpp
  ImfFlatImageLevel.cpp
  ImfDeepImageLevel.cpp
  ImfImage.cpp
  ImfFlatImage.cpp
  ImfDeepImage.cpp
  ImfImageIO.cpp
  ImfFlatImageIO.cpp
  ImfDeepImageIO.cpp
  ImfImageDataWindow.cpp
)

IF(BUILD_SHARED_LIBS)
    ADD_DEFINITIONS(-DILMIMF_EXPORTS)
ENDIF()

# LINK_DIRECTORIES ( ${CMAKE_CURRENT_BINARY_DIR}/../IlmImf )

ADD_LIBRARY ( openexr-ilmimfutil ${LIB_TYPE}
  ${ILMIMFUTIL_SRCS}
)

TARGET_LINK_LIBRARIES ( openexr-ilmimfutil 
  openexr-half
  openexr-iex
  openexr-imath
  openexr-ilmthread
  openexr-ilmimf
  # Half 
  # Iex${ILMBASE_LIBSUFFIX}
  # Imath${ILMBASE_LIBSUFFIX}
  # IlmThread${ILMBASE_LIBSUFFIX}
  # IlmImf
  ${PTHREAD_LIB} ${ZLIB_LIBRARIES}
)


# Libraries

# INSTALL ( TARGETS
#   IlmImfUtil
#   DESTINATION
#   ${CMAKE_INSTALL_PREFIX}/lib
# )

# # Headers
# INSTALL ( FILES
#   ImfImageChannel.h
#   ImfFlatImageChannel.h
#   ImfDeepImageChannel.h
#   ImfSampleCountChannel.h
#   ImfImageLevel.h
#   ImfFlatImageLevel.h
#   ImfDeepImageLevel.h
#   ImfImage.h
#   ImfFlatImage.h
#   ImfDeepImage.h
#   ImfImageIO.h
#   ImfFlatImageIO.h
#   ImfDeepImageIO.h
#   ImfImageDataWindow.h
#   ImfImageChannelRenaming.h
#   DESTINATION
#   ${CMAKE_INSTALL_PREFIX}/include/OpenEXR
#   )

#--------------------------------------------------------------------
# Supress warnings
ivw_suppress_compiler_warnings(openexr-ilmimfutil)

#--------------------------------------------------------------------
# Install library into target folder
ivw_default_install_targets(openexr-ilmimfutil)

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_folder(openexr-ilmimfutil ext/OpenEXR)
source_group("CMake Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_set_cpack_name(openexr-ilmimfutil)
ivw_make_package(openexr-ilmimfutil openexr-ilmimfutil)
