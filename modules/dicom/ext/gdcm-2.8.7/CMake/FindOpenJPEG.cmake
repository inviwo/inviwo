# - Try to find the OpenJPEG (JPEG 2000) library
#
# Read-Only variables:
#  OPENJPEG_FOUND - system has the OpenJPEG library
#  OPENJPEG_INCLUDE_DIR - the OpenJPEG include directory
#  OPENJPEG_LIBRARIES - The libraries needed to use OpenJPEG

#=============================================================================
# Copyright 2006-2011 Mathieu Malaterre <mathieu.malaterre@gmail.com>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

# Try with pkg-config first
find_package(PkgConfig)
pkg_check_modules(OPENJPEG libopenjp2)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenJPEG REQUIRED_VARS
  OPENJPEG_LIBRARIES
  OPENJPEG_INCLUDE_DIRS
  VERSION_VAR OPENJPEG_VERSION
)

mark_as_advanced(
  OPENJPEG_LIBRARIES
  OPENJPEG_INCLUDE_DIRS
  )
