#--------------------------------------------------------------------
# Add subdirectories
#add_subdirectory(zlib)
#add_subdirectory(libjpeg)
#add_subdirectory(libmng)
#add_subdirectory(libopenjpeg)
#add_subdirectory(libpng)
#add_subdirectory(librawlite)
#add_subdirectory(libtiff)
#add_subdirectory(libtiff4)
#add_subdirectory(openexr)
#--------------------------------------------------------------------
# FreeImage Package
ivw_project(freeimage)


#--------------------------------------------------------------------
# Supress warnings
if(WIN32)
add_definitions( "/W0 /D_CRT_SECURE_NO_WARNINGS /wd4005 /wd4996 /nologo /IGNORE:4006" )
endif()
#if(NOT DESIRED_QT_VERSION MATCHES 5)    
    # Certain zlib functions conflict with Qt4 or less. Hence Z_PREFIX is requried.
    #add_definitions( "/DZ_PREFIX" )
#endif()


#--------------------------------------------------------------------
# Add LIBJPEG files
file(GLOB_RECURSE LIBJPEG_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/libjpeg/*.h)
file(GLOB_RECURSE LIBJPEG_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/libjpeg/*.c)
list(REMOVE_ITEM LIBJPEG_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/libjpeg/ckconfig.c)
## macosx
list(REMOVE_ITEM LIBJPEG_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/libjpeg/jmemmac.c)
# This is kind of ugly, is there a better way? /Peter
if(${CMAKE_SYSTEM_NAME} MATCHES "(Darwin|FreeBSD)")
 ivw_include_directories(/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk/System/Library/Frameworks/CoreServices.framework/Frameworks/CarbonCore.framework/Headers/)
endif()

#--------------------------------------------------------------------
# Add LIBOpenJPEG files
file(GLOB_RECURSE LIBOPENJPEG_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/libopenjpeg/*.h)
file(GLOB_RECURSE LIBOPENJPEG_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/libopenjpeg/*.c)
list(REMOVE_ITEM LIBOPENJPEG_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/libopenjpeg/t1_generate_luts.c)

#--------------------------------------------------------------------
# Add LIBPNG files
file(GLOB_RECURSE LIBPNG_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/libpng/*.h)
file(GLOB_RECURSE LIBPNG_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/libpng/*.c)

#--------------------------------------------------------------------
# Add LIBRawLite files
file(GLOB_RECURSE LIBRAWLITE_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/librawlite/*.h)
list(REMOVE_ITEM LIBRAWLITE_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/librawlite/internal/libraw_internal_funcs.h)
file(GLOB_RECURSE LIBRAWLITE_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/librawlite/*.cpp)

#--------------------------------------------------------------------
# Add LIBTIFF4 files
file(GLOB_RECURSE LIBTIFF4_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/libtiff4/*.h)
file(GLOB_RECURSE LIBTIFF4_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/libtiff4/*.c)
list(REMOVE_ITEM LIBTIFF4_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/libtiff4/tif_config.wince.h)
list(REMOVE_ITEM LIBTIFF4_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/libtiff4/tif_wince.c)
list(REMOVE_ITEM LIBTIFF4_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/libtiff4/tif_win3.c)
list(REMOVE_ITEM LIBTIFF4_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/libtiff4/tif_vms.c)
list(REMOVE_ITEM LIBTIFF4_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/libtiff4/tif_win32.c)
list(REMOVE_ITEM LIBTIFF4_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/libtiff4/tif_apple.c)
list(REMOVE_ITEM LIBTIFF4_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/libtiff4/tif_unix.c)

if(NOT WIN32)
    list(REMOVE_ITEM LIBTIFF4_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/libtiff4/tiffconf.vc.h)
    list(REMOVE_ITEM LIBTIFF4_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/libtiff4/tif_config.vc.h)
endif()

#--------------------------------------------------------------------
# Add OpenEXR files
file(GLOB_RECURSE HALF_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/openexr/half/*.h)
set(HALF_SOURCE_FILES
    openexr/half/eLut.cpp
    openexr/half/half.cpp
    openexr/half/toFloat.cpp
)

file(GLOB_RECURSE IEX_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/openexr/iex/*.h)
file(GLOB_RECURSE IEX_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/openexr/iex/*.cpp)

file(GLOB_RECURSE ILLMIMF_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/openexr/ilmimf/*.h)
file(GLOB_RECURSE ILLMIMF_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/openexr/ilmimf/*.cpp)
list(REMOVE_ITEM ILLMIMF_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/openexr/ilmimf/b44ExpLogTable.cpp)

set(ILMTHREAD_HEADER_FILES
    openexr/ilmthread/IlmThread.h
    openexr/ilmthread/IlmThreadMutex.h
    openexr/ilmthread/IlmThreadPool.h
    openexr/ilmthread/IlmThreadSemaphore.h
)
if(WIN32)
set(ILMTHREAD_SOURCE_FILES
    openexr/ilmthread/IlmThreadPool.cpp
    openexr/ilmthread/IlmThreadPosix.cpp
    openexr/ilmthread/IlmThreadPool.cpp
    openexr/ilmthread/IlmThreadMutexWin32.cpp
    openexr/ilmthread/IlmThreadSemaphoreWin32.cpp
    openexr/ilmthread/IlmThreadWin32.cpp
)
else()
set(ILMTHREAD_SOURCE_FILES
    openexr/ilmthread/IlmThread.cpp
    openexr/ilmthread/IlmThreadMutex.cpp
    openexr/ilmthread/IlmThreadMutexPosix.cpp
    openexr/ilmthread/IlmThreadPool.cpp
    openexr/ilmthread/IlmThreadPosix.cpp
    openexr/ilmthread/IlmThreadPool.cpp
    openexr/ilmthread/IlmThreadSemaphore.cpp
    openexr/ilmthread/IlmThreadSemaphorePosix.cpp
    openexr/ilmthread/IlmThreadSemaphorePosixCompat.cpp
)
endif()

file(GLOB_RECURSE IMATH_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/openexr/imath/*.h)
file(GLOB_RECURSE IMATH_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/openexr/imath/*.cpp)

set(OPENEXR_HEADER_FILES ${HALF_HEADER_FILES} ${IEX_HEADER_FILES} ${ILLMIMF_HEADER_FILES} ${ILMTHREAD_HEADER_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/openexr/IlmBaseConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/openexr/OpenEXRConfig.h)
set(OPENEXR_SOURCE_FILES ${HALF_SOURCE_FILES} ${IEX_SOURCE_FILES} ${ILLMIMF_SOURCE_FILES} ${ILMTHREAD_SOURCE_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/openexr/asd.cpp)

#--------------------------------------------------------------------
# Add ZLib files
file(GLOB_RECURSE ZLIB_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/zlib/*.h)
file(GLOB_RECURSE ZLIB_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/zlib/*.c)

#--------------------------------------------------------------------
# Add header files

file(GLOB_RECURSE DeprecationManager_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/DeprecationManager/*.h)
file(GLOB_RECURSE FreeImage_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/FreeImage/*.h)
file(GLOB_RECURSE FreeImageToolkit_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/FreeImageToolkit/*.h)
file(GLOB_RECURSE Metadata_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Metadata/*.h)


set(General_HEADER_FILES 
                CacheFile.h
                FreeImage.h
                FreeImageIO.h
                Plugin.h
                Quantizers.h
                ToneMapping.h
                Utilities.h)

LIST(APPEND HEADER_FILES     ${DeprecationManager_HEADER_FILES} 
                            ${FreeImage_HEADER_FILES} 
                            ${FreeImageToolkit_HEADER_FILES} 
                            ${Metadata_HEADER_FILES}
                            ${General_HEADER_FILES}    )


ivw_group("Header Files" ${HEADER_FILES} ${LIBJPEG_HEADER_FILES} ${LIBOPENJPEG_HEADER_FILES} ${LIBPNG_HEADER_FILES} ${LIBRAWLITE_HEADER_FILES} ${LIBTIFF4_HEADER_FILES} ${OPENEXR_HEADER_FILES} ${ZLIB_HEADER_FILES})

#--------------------------------------------------------------------
# Add source files

file(GLOB_RECURSE DeprecationManager_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/DeprecationManager/*.cpp)
file(GLOB_RECURSE FreeImage_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/FreeImage/*.cpp)
file(GLOB_RECURSE FreeImageToolkit_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/FreeImageToolkit/*.cpp)
file(GLOB_RECURSE Metadata_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Metadata/*.cpp)


LIST(APPEND SOURCE_FILES  
                    ${DeprecationManager_SOURCE_FILES} 
                    ${FreeImage_SOURCE_FILES}
                    ${FreeImageToolkit_SOURCE_FILES}
                    ${Metadata_SOURCE_FILES}
        )

ivw_group("Source Files" ${SOURCE_FILES} ${LIBJPEG_SOURCE_FILES} ${LIBOPENJPEG_SOURCE_FILES} ${LIBPNG_SOURCE_FILES} ${LIBRAWLITE_SOURCE_FILES} ${LIBTIFF4_SOURCE_FILES} ${OPENEXR_SOURCE_FILES} ${ZLIB_SOURCE_FILES})

#--------------------------------------------------------------------
# Add all folders as includes
list_subdirectories(FREEIMAGE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} FALSE FALSE)

include_directories(${FREEIMAGE_DIRS})

#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(freeimage)
remove_definitions(-DHAVE_CONFIG_H)
if(NOT BUILD_SHARED_LIBS)
    ivw_add_definition(FREEIMAGE_LIB)
    ivw_add_definition(JPEG_STATIC)
    ivw_add_definition(OPJ_STATIC)
    ivw_add_definition(LIBRAW_NODLL)
else()
    add_definitions(-DJPEG_DLL)
    add_definitions(-DLIBRAW_BUILDLIB)
    add_definitions(-DOPJ_EXPORTS)
    add_definitions(-DHALF_EXPORTS)
endif()

#--------------------------------------------------------------------
# Create library
add_library(freeimage ${SOURCE_FILES} ${LIBJPEG_SOURCE_FILES} ${LIBOPENJPEG_SOURCE_FILES} ${LIBPNG_SOURCE_FILES} ${LIBRAWLITE_SOURCE_FILES} ${LIBTIFF4_SOURCE_FILES} ${OPENEXR_SOURCE_FILES} ${ZLIB_SOURCE_FILES} ${HEADER_FILES} ${LIBJPEG_HEADER_FILES} ${LIBOPENJPEG_HEADER_FILES} ${LIBPNG_HEADER_FILES} ${LIBRAWLITE_HEADER_FILES} ${LIBTIFF4_HEADER_FILES} ${OPENEXR_HEADER_FILES} ${ZLIB_HEADER_FILES})

#--------------------------------------------------------------------
# Add dependencies
#ivw_add_dependencies(
#    LibJPEG
#    LibOpenJPEG
#    InternalLibPNG  
#    LibRawLite 
#    LibTiff4
#    OpenEXR
#    InternalZLib
#    )

ivw_include_directories(${CMAKE_CURRENT_SOURCE_DIR})
ivw_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/FreeImage)
ivw_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libjpeg)
ivw_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libopenjpeg)
ivw_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libpng)
ivw_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/librawlite)
ivw_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libtiff4)
ivw_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/openexr/half)
ivw_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/openexr/imath)
ivw_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/openexr/ilmimf)
ivw_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/openexr/ilmthread)
ivw_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/openexr/iex)
ivw_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/openexr)
ivw_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/zlib)

#--------------------------------------------------------------------
# This is to use .lib file, should be deleted when real linker is working 
# link_directories(${CMAKE_CURRENT_SOURCE_DIR}/FreeImageLib/)

# ivw_add_dependency_libraries(FreeImage)

# SET_TARGET_PROPERTIES( freeimage PROPERTIES LINK_FLAGS
#                     "/LIBPATH:${CMAKE_CURRENT_SOURCE_DIR}/FreeImageLib/" )

# LIST(APPEND _allLibsDir "${CMAKE_CURRENT_SOURCE_DIR}/FreeImageLib" )

#---------------------------------------------------------------------

#--------------------------------------------------------------------
# Creates VS folder structure
#ivw_folder(freeimage ext/freeimagelib)
ivw_folder(freeimage ext)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_make_package(FreeImage freeimage)

if(CMAKE_SYSTEM_NAME MATCHES "(Darwin|FreeBSD)")
  #Ignore warnings in external packages
  set_target_properties(freeimage PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_INHIBIT_ALL_WARNINGS YES)
endif(CMAKE_SYSTEM_NAME MATCHES "(Darwin|FreeBSD)")

#--------------------------------------------------------------------
# Account for missing build flags in FreeImage
# otherwise FreeImage functionality will replace Qt Image functionality
#
# This fixes the "missing menu/toolbar icons" issue (Issue 540)
if(CMAKE_SYSTEM_NAME MATCHES "(Darwin|FreeBSD|Linux)")
  set (CMAKE_CXX_FLAGS "-fexceptions -fvisibility=hidden -fPIC")
  set (CMAKE_C_FLAGS "-fexceptions -fvisibility=hidden -fPIC")
endif(CMAKE_SYSTEM_NAME MATCHES "(Darwin|FreeBSD|Linux)")
    
