# yue.nicholas@gmail.com

IF(BUILD_SHARED_LIBS)
    ADD_DEFINITIONS(-DIEX_EXPORTS)
ENDIF()

ADD_LIBRARY ( openexr-iex ${LIB_TYPE}
  IexBaseExc.cpp
  IexThrowErrnoExc.cpp
  )


# INSTALL ( TARGETS
#   Iex
#   DESTINATION
#   lib
# )

# INSTALL ( FILES 
#   IexBaseExc.h
#   IexMathExc.h
#   IexThrowErrnoExc.h
#   IexErrnoExc.h
#   IexMacros.h
#   Iex.h
#   IexNamespace.h
#   IexExport.h
#   IexForward.h
#   DESTINATION
#   include/OpenEXR
# )

#--------------------------------------------------------------------
# Supress warnings
ivw_suppress_compiler_warnings(openexr-iex)

#--------------------------------------------------------------------
# Install library into target folder
ivw_default_install_targets(openexr-iex)

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_folder(openexr-iex ext/OpenEXR)
source_group("CMake Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_set_cpack_name(openexr-iex)
ivw_make_package(openexr-iex openexr-iex)
