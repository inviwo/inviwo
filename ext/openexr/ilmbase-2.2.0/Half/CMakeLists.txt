# yue.nicholas@gmail.com


IF(${OpenEXR_GENERATE_LUT})
  ADD_EXECUTABLE ( elut eLut.cpp )

  ADD_CUSTOM_COMMAND (
    TARGET elut POST_BUILD
    COMMAND elut > ${CMAKE_BINARY_DIR}/modules/_generated/ext/openexr/eLut.h
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/modules/_generated/ext/openexr
  )
  SET_SOURCE_FILES_PROPERTIES(
    ${CMAKE_BINARY_DIR}/modules/_generated/ext/openexr/eLut.h
    PROPERTIES HEADER_FILE_ONLY TRUE
    )

  ADD_EXECUTABLE ( tofloat toFloat.cpp )
  ADD_CUSTOM_COMMAND (
    TARGET tofloat POST_BUILD
    COMMAND tofloat > ${CMAKE_BINARY_DIR}/modules/_generated/ext/openexr/toFloat.h
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/modules/_generated/ext/openexr
  )
  SET_SOURCE_FILES_PROPERTIES(
    ${CMAKE_BINARY_DIR}/modules/_generated/ext/openexr/toFloat.h
    PROPERTIES HEADER_FILE_ONLY TRUE
    )
ENDIF()

SET_SOURCE_FILES_PROPERTIES(
  half.cpp
  PROPERTIES
  OBJECT_DEPENDS
  ${lookup_table_dir}/eLut.h
  OBJECT_DEPENDS
  ${lookup_table_dir}/toFloat.h
  )

IF(BUILD_SHARED_LIBS)
    ADD_DEFINITIONS(-DHALF_EXPORTS)
ENDIF()

ADD_LIBRARY ( openexr-half ${LIB_TYPE}
  half.cpp
  )

IF(${OpenEXR_GENERATE_LUT})
  ADD_DEPENDENCIES ( openexr-half tofloat elut )
ELSE()
  INCLUDE_DIRECTORIES(${lookup_table_dir})
ENDIF()

# INSTALL ( TARGETS
#   Half
#   DESTINATION
#   lib
# )

# INSTALL ( FILES 
#   half.h
#   halfFunction.h
#   halfExport.h
#   halfLimits.h
#   DESTINATION
#   include/OpenEXR
# )

#--------------------------------------------------------------------
# Supress warnings
ivw_suppress_compiler_warnings(openexr-half)

#--------------------------------------------------------------------
# Install library into target folder
ivw_default_install_targets(openexr-half)

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_folder(openexr-half ext/OpenEXR)
IF(${OpenEXR_GENERATE_LUT})
  ivw_folder(elut ext/OpenEXR/util)
  ivw_folder(tofloat ext/OpenEXR/util)
ENDIF()
source_group("CMake Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_set_cpack_name(openexr-half)
ivw_make_package(openexr-half openexr-half)
