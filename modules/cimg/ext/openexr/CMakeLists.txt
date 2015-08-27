#add_subdirectory(half)
#add_subdirectory(iex)
#add_subdirectory(ilmimf)
#add_subdirectory(ilmthread)
#add_subdirectory(imath)

ivw_project(openexr)

#--------------------------------------------------------------------
# Supress warnings
if(WIN32)
add_definitions( "/W0 /D_CRT_SECURE_NO_WARNINGS /wd4005 /wd4996 /nologo /IGNORE:4006" )
endif()

#--------------------------------------------------------------------
# Add Half files
file(GLOB_RECURSE HALF_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/half/*.h)
set(HALF_SOURCE_FILES
    half/eLut.cpp
    half/half.cpp
    half/toFloat.cpp
)

#--------------------------------------------------------------------
# Add Iex files
file(GLOB_RECURSE IEX_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/iex/*.h)
file(GLOB_RECURSE IEX_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/iex/*.cpp)

#--------------------------------------------------------------------
# Add Ilmimf files
file(GLOB_RECURSE ILLMIMF_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ilmimf/*.h)
file(GLOB_RECURSE ILLMIMF_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ilmimf/*.cpp)

#--------------------------------------------------------------------
# Add IlmThread files
set(ILMTHREAD_HEADER_FILES
    ilmthread/IlmThread.h
    ilmthread/IlmThreadMutex.h
    ilmthread/IlmThreadPool.h
    ilmthread/IlmThreadSemaphore.h
)
if(WIN32)
set(ILMTHREAD_SOURCE_FILES
    ilmthread/IlmThreadPool.cpp
    ilmthread/IlmThreadPosix.cpp
    ilmthread/IlmThreadPool.cpp
    ilmthread/IlmThreadMutexWin32.cpp
    ilmthread/IlmThreadSemaphoreWin32.cpp
    ilmthread/IlmThreadWin32.cpp
)
else()
set(ILMTHREAD_SOURCE_FILES
    ilmthread/IlmThread.cpp
    ilmthread/IlmThreadMutex.cpp
    ilmthread/IlmThreadMutexPosix.cpp
    ilmthread/IlmThreadPool.cpp
    ilmthread/IlmThreadPosix.cpp
    ilmthread/IlmThreadPool.cpp
    ilmthread/IlmThreadSemaphore.cpp
    ilmthread/IlmThreadSemaphorePosix.cpp
    ilmthread/IlmThreadSemaphorePosixCompat.cpp
)
endif()

#--------------------------------------------------------------------
# Add Imath files
file(GLOB_RECURSE IMATH_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/imath/*.h)
file(GLOB_RECURSE IMATH_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/imath/*.cpp)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES 	${CMAKE_CURRENT_SOURCE_DIR}/IlmBaseConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenEXRConfig.h)
ivw_group("Header Files" ${HEADER_FILES} ${HALF_HEADER_FILES} ${IEX_HEADER_FILES} ${ILLMIMF_HEADER_FILES} ${ILMTHREAD_HEADER_FILES} ${IMATH_HEADER_FILES})
	
#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES 	${CMAKE_CURRENT_SOURCE_DIR}/asd.cpp)
ivw_group("Source Files" ${SOURCE_FILES} ${HALF_SOURCE_FILES} ${IEX_SOURCE_FILES} ${ILLMIMF_SOURCE_FILES} ${ILMTHREAD_SOURCE_FILES} ${IMATH_SOURCE_FILES})

#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(openexr)
if(BUILD_SHARED_LIBS)
    ivw_add_definition(OPENEXR_DLL)
	add_definitions(-DHALF_EXPORTS)
    add_definitions(-DIEX_EXPORTS)
	add_definitions(-DILMIMF_EXPORTS)
	add_definitions(-DILMTHREAD_EXPORTS)
	add_definitions(-DIMATH_EXPORTS)
endif()

#--------------------------------------------------------------------
# Create library
#add_library(openexr ${SOURCE_FILES} ${HEADER_FILES})
add_library(openexr ${HEADER_FILES} ${SOURCE_FILES} ${HALF_HEADER_FILES} ${HALF_SOURCE_FILES} ${IEX_HEADER_FILES} ${IEX_SOURCE_FILES} ${ILLMIMF_HEADER_FILES} ${ILLMIMF_SOURCE_FILES} ${ILMTHREAD_HEADER_FILES} ${ILMTHREAD_SOURCE_FILES} ${IMATH_HEADER_FILES} ${IMATH_SOURCE_FILES})

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_folder(openexr ext/freeimagelib)

#--------------------------------------------------------------------
# Add dependencies
ivw_add_dependencies(InternalZLib)
#                  Half
#					Iex
#					IlmImf
#					IlmThread
#					IMath)
ivw_include_directories(${IVW_EXTENSIONS_DIR}/freeimage/openexr/half)
ivw_include_directories(${IVW_EXTENSIONS_DIR}/freeimage/openexr/imath)
ivw_include_directories(${IVW_EXTENSIONS_DIR}/freeimage/openexr/ilmimf)
ivw_include_directories(${IVW_EXTENSIONS_DIR}/freeimage/openexr/ilmthread)
ivw_include_directories(${IVW_EXTENSIONS_DIR}/freeimage/openexr/iex)
ivw_include_directories(${IVW_EXTENSIONS_DIR}/freeimage/openexr)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_make_package(OpenEXR openexr)

