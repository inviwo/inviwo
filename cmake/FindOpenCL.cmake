# FindOpenCL - attempts to locate the OpenCL library.
#
# This module defines the following variables:
#   OPENCL_FOUND        - Module was found
#   OPENCL_INCLUDE_DIR - Directory where cl.h is located
#   OPENCL_LIBRARIES    - Libraries necessary to compile 
#
# Note: Untested with Linux and AMD combinations
#=============================================================================
FIND_PACKAGE( PackageHandleStandardArgs )


# NVIDIA uses environment variable CUDA_PATH
# AMD uses AMDAPPSDKROOT according to http://developer.amd.com/download/AMD_APP_SDK_Installation_Notes.pdf
# INTEL uses environment variable INTELOCLSDKROOT

SET(OPENCL_LIB_SEARCH_DIR ${OPENCL_LIB_SEARCH_DIR} 
    "$ENV{LD_LIBRARY_PATH}") # Linux
    
# Find out build target 
IF(CMAKE_SIZEOF_VOID_P EQUAL 4) # 32-bit
    SET(OPENCL_LIB_SEARCH_DIR ${OPENCL_LIB_SEARCH_DIR}
        "$ENV{AMDAPPSDKROOT}/lib/x86"      # AMD
        "$ENV{CUDA_PATH}/lib/Win32"      # NVIDIA
        "$ENV{INTELOCLSDKROOT}/lib/x86") # INTEL
ELSE(CMAKE_SIZEOF_VOID_P EQUAL 4) # 64-bit
    SET(OPENCL_LIB_SEARCH_DIR ${OPENCL_LIB_SEARCH_DIR}
        "$ENV{AMDAPPSDKROOT}/lib/x86_64" # AMD
        "$ENV{CUDA_PATH}/lib/x64"        # NVIDIA
        "$ENV{INTELOCLSDKROOT}/lib/x64") # INTEL        
ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 4)

FIND_LIBRARY(OPENCL_LIBRARIES 
             NAMES OpenCL 
             PATHS ${OPENCL_LIB_SEARCH_DIR} 
             DOC "OpenCL library")
             
# Take care of custom install locations
GET_FILENAME_COMPONENT(OPENCL_LIB_SEARCH_DIR ${OPENCL_LIBRARIES} PATH)
# Optional search path
GET_FILENAME_COMPONENT(OPENCL_INCLUDE_SEARCH_DIR ${OPENCL_LIB_SEARCH_DIR}/../../include ABSOLUTE)

FIND_PATH(OPENCL_INCLUDE_DIR 
          NAMES CL/cl.h OpenCL/cl.h 
          PATHS 
                ${OPENCL_INCLUDE_SEARCH_DIR}
                "$ENV{AMDAPPSDKROOT}/include"   # AMD
                "$ENV{CUDA_PATH}/include"       # NVIDIA
                "$ENV{INTELOCLSDKROOT}/include" # INTEL
                "/usr/local/cuda/include"       # Linux NVIDIA
                "/opt/AMDAPP/include"           # Linux AMD 
          DOC "Include directory for OpenCL")
          
    
# Add quotes to paths to make sure it works on windows (at least Visual Studio complains)
#string(REPLACE " " "\\ " OPENCL_LIBRARIES ${OPENCL_LIBRARIES})
    


INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS( OpenCL DEFAULT_MSG OPENCL_LIBRARIES OPENCL_INCLUDE_DIR )

MARK_AS_ADVANCED(CLEAR
  OPENCL_INCLUDE_DIR
  OPENCL_LIBRARIES
)

