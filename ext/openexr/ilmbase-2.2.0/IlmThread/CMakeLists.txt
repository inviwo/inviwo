# yue.nicholas@gmail.com

SET( ILMTHREAD_LIBRARY_SOURCES
  IlmThread.cpp
  IlmThreadMutex.cpp
  IlmThreadMutexPosix.cpp
  IlmThreadPool.cpp
  IlmThreadPosix.cpp
  IlmThreadSemaphore.cpp
  IlmThreadSemaphorePosixCompat.cpp
  IlmThreadSemaphorePosix.cpp
)
IF (WIN32)
  SET( ILMTHREAD_LIBRARY_SOURCES ${ILMTHREAD_LIBRARY_SOURCES}
    IlmThreadMutexWin32.cpp
    IlmThreadSemaphoreWin32.cpp
    IlmThreadWin32.cpp
	)
ENDIF()


IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DILMTHREAD_EXPORTS)
ENDIF()

ADD_LIBRARY ( openexr-ilmthread ${LIB_TYPE}
  ${ILMTHREAD_LIBRARY_SOURCES}
  )
TARGET_LINK_LIBRARIES(openexr-ilmthread openexr-iex)

# INSTALL ( TARGETS
#   IlmThread
#   DESTINATION
#   lib
# )

# INSTALL ( FILES 
#   IlmThreadPool.h
#   IlmThread.h
#   IlmThreadSemaphore.h
#   IlmThreadMutex.h
#   IlmThreadNamespace.h
#   IlmThreadExport.h
#   IlmThreadForward.h
#   DESTINATION
#   include/OpenEXR
# )

#--------------------------------------------------------------------
# Supress warnings
ivw_suppress_compiler_warnings(openexr-ilmthread)

#--------------------------------------------------------------------
# Install library into target folder
ivw_default_install_targets(openexr-ilmthread)

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_folder(openexr-ilmthread ext/OpenEXR)
source_group("CMake Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_set_cpack_name(openexr-ilmthread)
ivw_make_package(openexr-ilmthread openexr-ilmthread)
