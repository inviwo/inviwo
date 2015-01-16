# - try to find glut library and include files
#  FREEGLUT_INCLUDE_DIR, where to find GL/glut.h, etc.
#  FREEGLUT_LIBRARIES, the libraries to link against
#  FREEGLUT_FOUND, If false, do not try to use Freeglut.
# Also defined, but not for general use are:
#  FREEGLUT_glut_LIBRARY = the full path to the glut library.
#  FREEGLUT_Xmu_LIBRARY  = the full path to the Xmu library.
#  FREEGLUT_Xi_LIBRARY   = the full path to the Xi Library.

#=============================================================================
# Copyright 2001-2009 Kitware, Inc.
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

IF (WIN32)
  FIND_PATH( FREEGLUT_INCLUDE_DIR NAMES GL/freeglut.h 
    PATHS  ${FREEGLUT_ROOT_PATH}/include )
  FIND_LIBRARY( FREEGLUT_glut_LIBRARY NAMES glut glut32 freeglut freeglut_static
    PATHS
    ${OPENGL_LIBRARY_DIR}
    ${FREEGLUT_ROOT_PATH}/Release
    )
ELSE (WIN32)
  
  IF (APPLE)
    FIND_LIBRARY(FREEGLUT_glut_LIBRARY 
		NAMES glut 
		PATHS "$ENV{LD_LIBRARY_PATH}"
		DOC "GLUT library for OSX") 
    FIND_PATH( FREEGLUT_INCLUDE_DIR NAMES GL/freeglut.h)
  ELSE (APPLE)
    
    FIND_PATH( FREEGLUT_INCLUDE_DIR GL/freeglut.h
      /usr/include/GL
      /usr/openwin/share/include
      /usr/openwin/include
      /opt/graphics/OpenGL/include
      /opt/graphics/OpenGL/contrib/libglut
      )
  
    FIND_LIBRARY( FREEGLUT_glut_LIBRARY glut
      /usr/openwin/lib
      )
    
    FIND_LIBRARY( FREEGLUT_Xi_LIBRARY Xi
      /usr/openwin/lib
      )
    
    FIND_LIBRARY( FREEGLUT_Xmu_LIBRARY Xmu
      /usr/openwin/lib
      )
    
  ENDIF (APPLE)
  
ENDIF (WIN32)

SET( FREEGLUT_FOUND "NO" )
IF(FREEGLUT_INCLUDE_DIR)
  IF(FREEGLUT_glut_LIBRARY)
    # Is -lXi and -lXmu required on all platforms that have it?
    # If not, we need some way to figure out what platform we are on.
    SET( FREEGLUT_LIBRARIES
      ${FREEGLUT_glut_LIBRARY}
      #${FREEGLUT_Xmu_LIBRARY}
      #${FREEGLUT_Xi_LIBRARY} 
      ${FREEGLUT_cocoa_LIBRARY}
      )
    SET( FREEGLUT_FOUND "YES" )
	GET_FILENAME_COMPONENT( FREEGLUT_LIBRARY_DIR ${FREEGLUT_glut_LIBRARY} PATH )
    
  ENDIF(FREEGLUT_glut_LIBRARY)
ENDIF(FREEGLUT_INCLUDE_DIR)

MARK_AS_ADVANCED(
  FREEGLUT_INCLUDE_DIR
  FREEGLUT_glut_LIBRARY
  FREEGLUT_Xmu_LIBRARY
  FREEGLUT_Xi_LIBRARY
  )
  
#=============================================================================
# InViWo Build
  
#--------------------------------------------------------------------
# Build FreeGLUT lib
if(NOT FREEGLUT_FOUND)
    set(BUILD_FREEGLUT 1)
	set(FREEGLUT_INCLUDE_DIR  ${IVW_MODULE_DIR}/glut/ext/freeglut/include)
	set(FREEGLUT_LIBRARY freeglut)
	set(FREEGLUT_LIBRARY_DIR ${LIBRARY_OUTPUT_PATH})
	mark_as_advanced(FORCE  FREEGLUT_INCLUDE_DIR )
	mark_as_advanced(FORCE  FREEGLUT_LIBRARY )
    
    if(WIN32 AND BUILD_SHARED_LIBS)
        set(FREEGLUT_LIBRARIES optimized ${IVW_LIBRARY_DIR}/Release/${FREEGLUT_LIBRARY}.lib debug ${IVW_LIBRARY_DIR}/Debug/${FREEGLUT_LIBRARY}${CMAKE_DEBUG_POSTFIX}.lib)
    else()
        set(FREEGLUT_LIBRARIES optimized ${FREEGLUT_LIBRARY} debug ${FREEGLUT_LIBRARY}${CMAKE_DEBUG_POSTFIX})
    endif()
    
endif(NOT FREEGLUT_FOUND)
