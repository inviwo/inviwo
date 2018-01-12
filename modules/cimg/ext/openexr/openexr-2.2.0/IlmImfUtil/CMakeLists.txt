
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

TARGET_LINK_LIBRARIES ( openexr-ilmimfutil PUBLIC
  openexr-half
  openexr-iex
  openexr-imath
  openexr-ilmthread
  openexr-ilmimf
  ZLIB::ZLIB
  # Half 
  # Iex${ILMBASE_LIBSUFFIX}
  # Imath${ILMBASE_LIBSUFFIX}
  # IlmThread${ILMBASE_LIBSUFFIX}
  # IlmImf
  ${PTHREAD_LIB}
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

target_include_directories(openexr-ilmimfutil PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

#--------------------------------------------------------------------
# Install library into target folder
ivw_default_install_comp_targets(core openexr-ilmimfutil)

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_folder(openexr-ilmimfutil ext/OpenEXR)
source_group("CMake Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_make_package(openexr-ilmimfutil openexr-ilmimfutil)
