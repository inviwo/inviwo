# - this module looks for Matlab
# Defines:
#  MATLAB_INCLUDE_DIR: include path for mex.h, engine.h
#  MATLAB_LIBRARIES:   required libraries: libmex, etc
#  MATLAB_MEX_LIBRARY: path to libmex.lib
#  MATLAB_MX_LIBRARY:  path to libmx.lib
#  MATLAB_ENG_LIBRARY: path to libeng.lib

#=============================================================================
# Copyright 2005-2009 Kitware, Inc.
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

set(MATLAB_FOUND 0)
if(WIN32)  
   
   #This is not working hence hardcoded for now.
   #set(MATLAB_ROOT "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MathWorks\\MATLAB\\7.13;MATLABROOT]/extern/lib/win32/microsoft/msvc60")
    message(STATUS "CMAKE_GENERATOR: " ${CMAKE_GENERATOR})
    if (${CMAKE_GENERATOR} MATCHES "Visual Studio 11 Win64")
	  set(MATLAB_ROOT "C:/Program Files/MATLAB/R2011b")
      # Assume people are generally using 7.1,
      # if using 7.0 need to link to: ../extern/lib/win32/microsoft/msvc70	  
      set(MATLAB_LIB_PATH "${MATLAB_ROOT}/extern/lib/win64/microsoft")
	  message(STATUS "MATLAB_ROOT: " ${MATLAB_ROOT})
    elseif ( ${CMAKE_GENERATOR} MATCHES "Visual Studio 9 2008")
	  set(MATLAB_ROOT "C:/Program Files (x86)/MATLAB/R2011b")
      # Assume people are generally using 7.1,
      # if using 7.0 need to link to: ../extern/lib/win32/microsoft/msvc70	  
      set(MATLAB_LIB_PATH "${MATLAB_ROOT}/extern/lib/win32/microsoft")
	  message(STATUS "MATLAB_ROOT: " ${MATLAB_ROOT})
    else()
        if(CUSTOM_MATLAB_FIND_REQUIRED)
          message(FATAL_ERROR "Generator not compatible: ${CMAKE_GENERATOR}")
        endif()     
    endif()
  
  find_library(MATLAB_MEX_LIBRARY
    libmex
    ${MATLAB_LIB_PATH}
    )
  find_library(MATLAB_MX_LIBRARY
    libmx
    ${MATLAB_LIB_PATH}
    )
  find_library(MATLAB_ENG_LIBRARY
    libeng
    ${MATLAB_LIB_PATH}
    )
	
 find_library(MATLAB_MAT_LIBRARY
    libmat
    ${MATLAB_LIB_PATH}
    )

 find_library(MATLAB_EMLRT_LIBRARY
    libemlrt
    ${MATLAB_LIB_PATH}
    )   
	
  find_path(MATLAB_INCLUDE_DIR
    "mex.h"
    "${MATLAB_ROOT}/extern/include" 	
    )
endif()

# This is common to UNIX and Win32:
set(MATLAB_LIBRARIES
  ${MATLAB_MEX_LIBRARY}
  ${MATLAB_MX_LIBRARY}
  ${MATLAB_ENG_LIBRARY}
  ${MATLAB_MAT_LIBRARY}
  ${MATLAB_EMLRT_LIBRARY}
)

if(MATLAB_INCLUDE_DIR AND MATLAB_LIBRARIES)
  set(MATLAB_FOUND 1)
endif()

mark_as_advanced(
  MATLAB_LIBRARIES
  MATLAB_MEX_LIBRARY
  MATLAB_MX_LIBRARY
  MATLAB_ENG_LIBRARY
  MATLAB_MAT_LIBRARY
  MATLAB_EMLRT_LIBRARY
  MATLAB_INCLUDE_DIR
  MATLAB_FOUND
  MATLAB_ROOT
)
