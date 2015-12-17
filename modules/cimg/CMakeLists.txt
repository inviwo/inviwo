#if(IVW_MODULE_CIMG)
#    add_subdirectory(ext/libpng)
#    add_subdirectory(ext/libjpeg)
#endif(IVW_MODULE_CIMG)

#--------------------------------------------------------------------
# CImg Module
ivw_module(CImg)

#--------------------------------------------------------------------
# Supress warnings
if(WIN32)
add_definitions( "/W0 /D_CRT_SECURE_NO_WARNINGS /wd4005 /wd4996 /nologo /IGNORE:4006" )
endif()

#--------------------------------------------------------------------
# Add LIBJPEG files
file(GLOB_RECURSE LIBJPEG_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/libjpeg/*.h)
file(GLOB_RECURSE LIBJPEG_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/libjpeg/*.c)
list(REMOVE_ITEM LIBJPEG_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/libjpeg/ckconfig.c)
## macosx
list(REMOVE_ITEM LIBJPEG_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/libjpeg/jmemmac.c)
# This is kind of ugly, is there a better way? /Peter
if(${CMAKE_SYSTEM_NAME} MATCHES "(Darwin|FreeBSD)")
 ivw_include_directories(/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk/System/Library/Frameworks/CoreServices.framework/Frameworks/CarbonCore.framework/Headers/)
endif()

#--------------------------------------------------------------------
# Add LIBPNG files
file(GLOB_RECURSE LIBPNG_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/libpng/*.h)
file(GLOB_RECURSE LIBPNG_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/libpng/*.c)

#--------------------------------------------------------------------
# Add OpenEXR files
file(GLOB_RECURSE HALF_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/half/*.h)
set(HALF_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/half/eLut.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/half/half.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/half/toFloat.cpp
)

file(GLOB_RECURSE IEX_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/iex/*.h)
file(GLOB_RECURSE IEX_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/iex/*.cpp)

file(GLOB_RECURSE ILLMIMF_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/ilmimf/*.h)
file(GLOB_RECURSE ILLMIMF_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/ilmimf/*.cpp)
list(REMOVE_ITEM ILLMIMF_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/ilmimf/b44ExpLogTable.cpp)

set(ILMTHREAD_HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/ilmthread/IlmThread.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/ilmthread/IlmThreadMutex.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/ilmthread/IlmThreadPool.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/ilmthread/IlmThreadSemaphore.h
)
if(WIN32)
set(ILMTHREAD_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/ilmthread/IlmThreadPool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/ilmthread/IlmThreadPosix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/ilmthread/IlmThreadPool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/ilmthread/IlmThreadMutexWin32.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/ilmthread/IlmThreadSemaphoreWin32.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/ilmthread/IlmThreadWin32.cpp
)
else()
set(ILMTHREAD_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/ilmthread/IlmThread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/ilmthread/IlmThreadMutex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/ilmthread/IlmThreadMutexPosix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/ilmthread/IlmThreadPool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/ilmthread/IlmThreadPosix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/ilmthread/IlmThreadPool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/ilmthread/IlmThreadSemaphore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/ilmthread/IlmThreadSemaphorePosix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/ilmthread/IlmThreadSemaphorePosixCompat.cpp
)
endif()

file(GLOB_RECURSE IMATH_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/imath/*.h)
file(GLOB_RECURSE IMATH_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/imath/*.cpp)

set(OPENEXR_HEADER_FILES 
    ${HALF_HEADER_FILES} 
    ${IEX_HEADER_FILES} 
    ${ILLMIMF_HEADER_FILES} 
    ${ILMTHREAD_HEADER_FILES} 
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/IlmBaseConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/OpenEXRConfig.h)
set(OPENEXR_SOURCE_FILES 
    ${HALF_SOURCE_FILES} 
    ${IEX_SOURCE_FILES} 
    ${ILLMIMF_SOURCE_FILES} 
    ${ILMTHREAD_SOURCE_FILES} 
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/asd.cpp)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/cimg/CImg.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cimgutils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cimglayerreader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cimglayerwriter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cimgvolumereader.h
)
ivw_group("Header Files" ${HEADER_FILES} 
    ${LIBJPEG_HEADER_FILES} 
    ${LIBPNG_HEADER_FILES} 
    ${LIBRAWLITE_HEADER_FILES} 
    ${OPENEXR_HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/cimgutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cimglayerreader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cimglayerwriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cimgvolumereader.cpp
)
ivw_group("Source Files" ${SOURCE_FILES} 
    ${LIBJPEG_SOURCE_FILES} 
    ${LIBPNG_SOURCE_FILES}
    ${OPENEXR_SOURCE_FILES})

#--------------------------------------------------------------------
# Add all folders as includes
list_subdirectories(LIB_IMAGE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/ext FALSE FALSE)
include_directories(${LIB_IMAGE_DIRS})

#--------------------------------------------------------------------
# Define defintions
#if(OPENMP_ON)
#ivw_add_definition(cimg_use_openmp)
#endif()
ivw_add_definition(cimg_use_cpp11)
ivw_add_definition(cimg_use_png)
ivw_add_definition(cimg_use_jpeg)
if (ZLIB_FOUND)
	ivw_add_definition(cimg_use_zlib)
endif()
ivw_add_definition(cimg_use_openexr)

remove_definitions(-DHAVE_CONFIG_H)
if(NOT BUILD_SHARED_LIBS)
    ivw_add_definition(JPEG_STATIC)
else()
    add_definitions(-DJPEG_DLL)
    add_definitions(-DHALF_EXPORTS)
endif()

#--------------------------------------------------------------------
# Add dependencies
#ivw_add_dependencies(
#    LibJPEG
#    InternalLibPNG  
#    OpenEXR
#    InternalZLib
#    )

ivw_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext)
ivw_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext/libjpeg)
ivw_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext/libpng)
ivw_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/half)
ivw_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/imath)
ivw_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/ilmimf)
ivw_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/ilmthread)
ivw_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/iex)
ivw_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr)

#--------------------------------------------------------------------
# Supress warning from ext...
set(IGNORE_WARN_FILES 
    ${LIBJPEG_SOURCE_FILES} 
    ${LIBPNG_SOURCE_FILES}
    ${OPENEXR_SOURCE_FILES}
)

if(MSVC)
    set_source_files_properties( ${IGNORE_WARN_FILES} PROPERTIES COMPILE_FLAGS " /W0 " )
endif(MSVC)
if(CMAKE_SYSTEM_NAME MATCHES "(Darwin|FreeBSD)")
    set_source_files_properties(${IGNORE_WARN_FILES} PROPERTIES COMPILE_FLAGS " -w ")
endif(CMAKE_SYSTEM_NAME MATCHES "(Darwin|FreeBSD)")


#--------------------------------------------------------------------
# Create module
ivw_create_module(
    ${SOURCE_FILES} 
    ${LIBJPEG_SOURCE_FILES} 
    ${LIBPNG_SOURCE_FILES}
    ${OPENEXR_SOURCE_FILES}
    ${HEADER_FILES}
    ${LIBJPEG_HEADER_FILES}
    ${LIBPNG_HEADER_FILES}
    ${OPENEXR_HEADER_FILES}
)