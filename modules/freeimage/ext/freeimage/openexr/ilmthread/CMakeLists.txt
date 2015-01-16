ivw_project(ilmthread)

#--------------------------------------------------------------------
# Supress warnings
if(WIN32)
add_definitions( "/W0 /D_CRT_SECURE_NO_WARNINGS /wd4005 /wd4996 /nologo /IGNORE:4006" )
endif()

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    IlmThread.h
    IlmThreadMutex.h
    IlmThreadPool.h
    IlmThreadSemaphore.h
)
ivw_group("IlmThread Header Files" ${HEADER_FILES} )

#--------------------------------------------------------------------
# Add source files
if(WIN32)
set(SOURCE_FILES
    IlmThreadPool.cpp
    IlmThreadPosix.cpp
    IlmThreadPool.cpp
    IlmThreadMutexWin32.cpp
    IlmThreadSemaphoreWin32.cpp
    IlmThreadWin32.cpp
)
else()
set(SOURCE_FILES
    IlmThread.cpp
    IlmThreadMutex.cpp
    IlmThreadMutexPosix.cpp
    IlmThreadPool.cpp
    IlmThreadPosix.cpp
    IlmThreadPool.cpp
    IlmThreadSemaphore.cpp
    IlmThreadSemaphorePosix.cpp
    IlmThreadSemaphorePosixCompat.cpp
)
endif()

ivw_group("IlmThread Source Files" ${SOURCE_FILES} )

#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(ilmthread)
if(BUILD_SHARED_LIBS)
    ivw_add_definition(OPENEXR_DLL)
endif()

#--------------------------------------------------------------------
# Create library
add_library(ilmthread ${SOURCE_FILES} ${HEADER_FILES})

#--------------------------------------------------------------------
# Creates VS folder structure
ivw_vs_folder(ilmthread ext/freeimagelib/openexrlib)

#--------------------------------------------------------------------
# Add dependencies
ivw_add_dependencies(Half
					Iex
                    Imath)
#ivw_include_directories(${IVW_EXTENSIONS_DIR}/freeimage/openexr/)
#ivw_include_directories(${IVW_EXTENSIONS_DIR}/freeimage/openexr/iex)

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_include_directories(${IVW_EXTENSIONS_DIR}/freeimage/openexr/)
ivw_include_directories(${IVW_EXTENSIONS_DIR}/freeimage/openexr/ilmthread)
ivw_make_package(IlmThread ilmthread)