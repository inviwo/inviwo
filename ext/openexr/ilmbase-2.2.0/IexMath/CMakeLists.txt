# pstanczyk@ilm.com

IF(BUILD_SHARED_LIBS)
    ADD_DEFINITIONS(-DIEXMATH_EXPORTS)
ENDIF()
ADD_LIBRARY ( openexr-iexmath ${LIB_TYPE}
  IexMathFloatExc.cpp  
  IexMathFpu.cpp
  )
TARGET_LINK_LIBRARIES(openexr-iexmath openexr-iex)

# INSTALL ( TARGETS
#   IexMath
#   DESTINATION
#   lib
# )

# INSTALL ( FILES 
#   IexMathFloatExc.h  
#   IexMathFpu.h  
#   IexMathIeeeExc.h
#   DESTINATION
#   include/OpenEXR
# )

#--------------------------------------------------------------------
# Supress warnings
ivw_suppress_compiler_warnings(openexr-iexmath)

target_include_directories(openexr-iexmath PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)


#--------------------------------------------------------------------
# Install library into target folder
ivw_default_install_comp_targets(core openexr-iexmath)

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_folder(openexr-iexmath ext/OpenEXR)
source_group("CMake Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_make_package(openexr-iexmath openexr-iexmath)
