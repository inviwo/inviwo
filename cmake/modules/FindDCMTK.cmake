
#=============================================================================
# InViWo Build
  
#--------------------------------------------------------------------
# Build DICOM lib
SET( DCMTK_FOUND "NO")

if(NOT DCMTK_FOUND)
  set(BUILD_DCMTK 1)
  # set(DICOM_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/modules/glut/ext/freeglut/include)
  set(DCMTK_LIBRARY DCMTK)
  set(DCMTK_LIBRARY_DIR ${LIBRARY_OUTPUT_PATH})
  mark_as_advanced(FORCE  DCMTK_INCLUDE_DIR )
  mark_as_advanced(FORCE  DCMTK_LIBRARY )
  set(DCMTK_LIBRARIES optimized ${DCMTK_LIBRARY} debug ${DCMTK_LIBRARY}${CMAKE_DEBUG_POSTFIX})
endif(NOT DCMTK_FOUND)
