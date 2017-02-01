# yue.nicholas@gmail.com

IF(BUILD_SHARED_LIBS)
    ADD_DEFINITIONS(-DIMATH_EXPORTS)
ENDIF()

ADD_LIBRARY ( openexr-imath ${LIB_TYPE}
  ImathBox.cpp
  ImathRandom.cpp
  ImathColorAlgo.cpp
  ImathShear.cpp
  ImathFun.cpp
  ImathVec.cpp
  ImathMatrixAlgo.cpp
)
TARGET_LINK_LIBRARIES(openexr-imath openexr-iex)


# INSTALL ( TARGETS
#   Imath
#   DESTINATION
#   lib
# )

# INSTALL ( FILES 
#   ImathBoxAlgo.h
#   ImathBox.h
#   ImathColorAlgo.h
#   ImathColor.h
#   ImathEuler.h
#   ImathExc.h
#   ImathExport.h
#   ImathForward.h
#   ImathFrame.h
#   ImathFrustum.h
#   ImathFrustumTest.h
#   ImathFun.h
#   ImathGL.h
#   ImathGLU.h
#   ImathHalfLimits.h
#   ImathInt64.h
#   ImathInterval.h
#   ImathLimits.h
#   ImathLineAlgo.h
#   ImathLine.h
#   ImathMath.h
#   ImathMatrixAlgo.h
#   ImathMatrix.h
#   ImathNamespace.h
#   ImathPlane.h
#   ImathPlatform.h
#   ImathQuat.h
#   ImathRandom.h
#   ImathRoots.h
#   ImathShear.h
#   ImathSphere.h
#   ImathVecAlgo.h
#   ImathVec.h
#   DESTINATION
#   include/OpenEXR
# )

#--------------------------------------------------------------------
# Supress warnings
ivw_suppress_compiler_warnings(openexr-imath)

#--------------------------------------------------------------------
# Install library into target folder
ivw_default_install_targets(openexr-imath)

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_folder(openexr-imath ext/OpenEXR)
source_group("CMake Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_set_cpack_name(openexr-imath)
ivw_make_package(openexr-imath openexr-imath)
