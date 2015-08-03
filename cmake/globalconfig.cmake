 #################################################################################
 #
 # Inviwo - Interactive Visualization Workshop
 #
 # Copyright (c) 2013-2015 Inviwo Foundation
 # All rights reserved.
 # 
 # Redistribution and use in source and binary forms, with or without
 # modification, are permitted provided that the following conditions are met: 
 # 
 # 1. Redistributions of source code must retain the above copyright notice, this
 # list of conditions and the following disclaimer. 
 # 2. Redistributions in binary form must reproduce the above copyright notice,
 # this list of conditions and the following disclaimer in the documentation
 # and/or other materials provided with the distribution. 
 # 
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 # ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 # WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 # DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 # ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 # (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 # LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 # ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 # (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 # SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 # 
 #################################################################################
 
set(IVW_VERSION 0.9.3dev)
set(IVW_MAJOR_VERSION 0)
set(IVW_MINOR_VERSION 9)
set(IVW_PATCH_VERSION 3dev)
 
set_property(GLOBAL PROPERTY USE_FOLDERS On)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER cmake)

#--------------------------------------------------------------------
# Only output error messages
function(message)
if( GET )
  list(GET ARGV 0 MessageType)
  if(MessageType STREQUAL FATAL_ERROR OR
     MessageType STREQUAL SEND_ERROR OR
     MessageType STREQUAL WARNING OR
     MessageType STREQUAL AUTHOR_WARNING)
    list(REMOVE_AT ARGV 0)
    _message(STATUS "${ARGV}")
  endif()
endif()
endfunction()
function(ivw_message)
    _message(${ARGV})
endfunction()

#--------------------------------------------------------------------
# Add own cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_BINARY_DIR}/cmake/")

include(${CMAKE_CURRENT_LIST_DIR}/globalmacros.cmake)

#--------------------------------------------------------------------
# Add parameter for paths to external modules
set(IVW_EXTERNAL_MODULES "" CACHE STRING "Paths to directory containing external modules")

#--------------------------------------------------------------------
# Add parameter for paths to external projects
set(IVW_EXTERNAL_PROJECTS "" CACHE STRING "Paths to directory containing external projects")

#--------------------------------------------------------------------
# Output paths for the executables, runtimes, archives and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin CACHE PATH
   "Single Directory for all Executables.")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib CACHE PATH
   "Single Directory for all static libraries.")
   
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib CACHE PATH
   "Single Directory for all Libraries")

mark_as_advanced(
    CMAKE_RUNTIME_OUTPUT_DIRECTORY
    CMAKE_BUNDLE_OUTPUT_DIRECTORY 
    CMAKE_ARCHIVE_OUTPUT_DIRECTORY
    CMAKE_LIBRARY_OUTPUT_DIRECTORY
)

#--------------------------------------------------------------------
# Path for this solution
if(NOT EXECUTABLE_OUTPUT_PATH)
  set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE PATH 
    "Single output directory for building all executables.")
endif(NOT EXECUTABLE_OUTPUT_PATH)

if(NOT LIBRARY_OUTPUT_PATH)
  set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib CACHE PATH
    "Single output directory for building all libraries.")
endif(NOT LIBRARY_OUTPUT_PATH)

mark_as_advanced(EXECUTABLE_OUTPUT_PATH LIBRARY_OUTPUT_PATH)

# Set Common Variables
get_filename_component(IVW_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR} PATH)
set(IVW_INCLUDE_DIR             ${IVW_ROOT_DIR}/include)
set(IVW_CORE_INCLUDE_DIR        ${IVW_ROOT_DIR}/include/inviwo/core)
set(IVW_QT_INCLUDE_DIR          ${IVW_ROOT_DIR}/include/inviwo/qt)
set(IVW_MODULE_DIR              ${IVW_ROOT_DIR}/modules)
set(IVW_SOURCE_DIR              ${IVW_ROOT_DIR}/src)
set(IVW_CORE_SOURCE_DIR         ${IVW_ROOT_DIR}/src/core)
set(IVW_QT_SOURCE_DIR           ${IVW_ROOT_DIR}/src/qt)
set(IVW_APPLICATION_DIR         ${IVW_ROOT_DIR}/apps)
set(IVW_RESOURCES_DIR           ${IVW_ROOT_DIR}/resources)
set(IVW_EXTENSIONS_DIR          ${IVW_ROOT_DIR}/ext)
set(IVW_BINARY_DIR              ${CMAKE_BINARY_DIR})
set(IVW_LIBRARY_DIR             ${LIBRARY_OUTPUT_PATH})
set(IVW_EXECUTABLE_DIR          ${EXECUTABLE_OUTPUT_PATH})
set(IVW_CMAKE_SOURCE_MODULE_DIR ${CMAKE_CURRENT_LIST_DIR})
set(IVW_CMAKE_BINARY_MODULE_DIR ${CMAKE_BINARY_DIR}/cmake)

#Generate headers
generate_external_module_header()
configure_file(${IVW_CMAKE_SOURCE_MODULE_DIR}/inviwocommondefines_template.h 
               ${CMAKE_BINARY_DIR}/modules/_generated/inviwocommondefines.h 
               @ONLY IMMEDIATE)

# Set ignored libs
set(VS_MULTITHREADED_DEBUG_DLL_IGNORE_LIBRARY_FLAGS
    "/NODEFAULTLIB:libc.lib
     /NODEFAULTLIB:libcmt.lib
     /NODEFAULTLIB:msvcrt.lib
     /NODEFAULTLIB:libcd.lib
     /NODEFAULTLIB:libcmtd.lib"
)
set(VS_MULTITHREADED_RELEASE_DLL_IGNORE_LIBRARY_FLAGS
    "/NODEFAULTLIB:libc.lib
     /NODEFAULTLIB:libcmt.lib
     /NODEFAULTLIB:libcd.lib
     /NODEFAULTLIB:libcmtd.lib
     /NODEFAULTLIB:msvcrtd.lib"
)
    
#--------------------------------------------------------------------
# Disable deprecation warnings for standard C functions
if(CMAKE_COMPILER_2005)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
endif(CMAKE_COMPILER_2005)

# Mac specific
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_definitions(-DDARWIN)
endif()

option(IVW_USE_GLFW_NOT_OPENGLQT "Use GLFW for context creation instead of OpenGLQt module" OFF)
mark_as_advanced(FORCE IVW_USE_GLFW_NOT_OPENGLQT)

#--------------------------------------------------------------------
# Package creation
option(IVW_PACKAGE_PROJECT "Create Inviwo Package Project" OFF)

#--------------------------------------------------------------------
# Use and generate resources when available
option(IVW_USE_GENERATED_RESOURCES "Use and Generate File Resources" OFF)

# Build shared libs or static libs
option(IVW_PROFILING "Enable profiling" OFF)
option(IVW_ENABLE_MSVC_MEMLEAK_TEST "Run memoryleak test within Visual Studio" OFF)
#mark_as_advanced(SHARED_LIBS)
mark_as_advanced(BUILD_SHARED_LIBS)
mark_as_advanced(FORCE GLM_DIR)

mark_as_advanced(FORCE CMAKE_CONFIGURATION_TYPES)

if(SHARED_LIBS)
    set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libs, else static libs" FORCE)
    if(WIN32)
        if(MSVC)
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
            set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd /Zi")
            set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MD")
            set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MDd /Zi")
        endif(MSVC)
    endif()
else()
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libs, else static libs" FORCE)
    if(WIN32)
        if(MSVC)
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
        endif(MSVC)
    endif()
endif()


#--------------------------------------------------------------------
# Disable deprecation warnings for standard C functions
if(WIN32 AND MSVC)
    add_definitions( "/W3 /D_CRT_SECURE_NO_WARNINGS /wd4005 /wd4996 /nologo" )
    string(REGEX REPLACE "[/\\-]Zm[0-9]+" " " CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Ym0x20000000")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zm512")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif()


#--------------------------------------------------------------------
# MSVC Variable checks and include redist in packs
if(WIN32)
    if(MSVC)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            if(MSVC_VERSION GREATER 1599)
              # VS 10 and later:
              set(CMAKE_MSVC_ARCH x64)
            else()
              # VS 9 and earlier:
              set(CMAKE_MSVC_ARCH amd64)
            endif()
        else()
            set(CMAKE_MSVC_ARCH x86)
        endif()
        if(MSVC90)
            set(MSVC_ACRO "9")
        elseif(MSVC10)
            set(MSVC_ACRO "10")
        elseif(MSVC11)
            set(MSVC_ACRO "11")
        elseif(MSVC12)
            set(MSVC_ACRO "12")
        endif()
        if(IVW_PACKAGE_PROJECT AND BUILD_SHARED_LIBS)
            if(DEFINED MSVC_ACRO)
                install(FILES "C:/Program Files (x86)/Microsoft Visual Studio ${MSVC_ACRO}.0/VC/redist/Debug_NonRedist/${CMAKE_MSVC_ARCH}/Microsoft.VC${MSVC_ACRO}0.DebugCRT/msvcp${MSVC_ACRO}0d.dll" DESTINATION bin COMPONENT core CONFIGURATIONS Debug)
                install(FILES "C:/Program Files (x86)/Microsoft Visual Studio ${MSVC_ACRO}.0/VC/redist/Debug_NonRedist/${CMAKE_MSVC_ARCH}/Microsoft.VC${MSVC_ACRO}0.DebugCRT/msvcr${MSVC_ACRO}0d.dll" DESTINATION bin COMPONENT core CONFIGURATIONS Debug)
                install(FILES "C:/Program Files (x86)/Microsoft Visual Studio ${MSVC_ACRO}.0/VC/redist/${CMAKE_MSVC_ARCH}/Microsoft.VC${MSVC_ACRO}0.CRT/msvcp${MSVC_ACRO}0.dll" DESTINATION bin COMPONENT core CONFIGURATIONS Release)
                install(FILES "C:/Program Files (x86)/Microsoft Visual Studio ${MSVC_ACRO}.0/VC/redist/${CMAKE_MSVC_ARCH}/Microsoft.VC${MSVC_ACRO}0.CRT/msvcr${MSVC_ACRO}0.dll" DESTINATION bin COMPONENT core CONFIGURATIONS Release)
            endif()
        endif()
    endif()
endif()

#--------------------------------------------------------------------
# Check if OpenMP is available and set it to use, and include the dll in packs
find_package(OpenMP QUIET)
if(OPENMP_FOUND)
    option(OPENMP_ON "Use OpenMP" ON)
    if(OPENMP_ON)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
        if(IVW_PACKAGE_PROJECT AND BUILD_SHARED_LIBS)
            if(WIN32)
                if(MSVC)
                    if(DEFINED MSVC_ACRO)
                        install(FILES "C:/Program Files (x86)/Microsoft Visual Studio ${MSVC_ACRO}.0/VC/redist/Debug_NonRedist/${CMAKE_MSVC_ARCH}/Microsoft.VC${MSVC_ACRO}0.DebugOpenMP/vcomp${MSVC_ACRO}0d.dll" DESTINATION bin COMPONENT core CONFIGURATIONS Debug)
                        install(FILES "C:/Program Files (x86)/Microsoft Visual Studio ${MSVC_ACRO}.0/VC/redist/${CMAKE_MSVC_ARCH}/Microsoft.VC${MSVC_ACRO}0.OPENMP/vcomp${MSVC_ACRO}0.dll" DESTINATION bin COMPONENT core CONFIGURATIONS Release)
                    endif()
                endif()
            endif()
        endif()
    endif()
endif()

#--------------------------------------------------------------------
# Set preprocessor definition to indicate whether 
# to use the debug postfix
# Add debug postfix if WIN32
IF(WIN32)
    IF(MSVC)
        SET(CMAKE_DEBUG_POSTFIX "d")
       
        option(MULTI_PROCESSOR_BUILD "Build with multiple processors" ON)
        if(MULTI_PROCESSOR_BUILD)
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
            SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
        endif()    
    ENDIF(MSVC)
ENDIF()

if(DEBUG_POSTFIX)
    add_definitions(-D_DEBUG_POSTFIX)
endif(DEBUG_POSTFIX)

option(PRECOMPILED_HEADERS "Create and use precompilied headers" ON)

#--------------------------------------------------------------------
# Specify build-based defintions
if(BUILD_SHARED_LIBS)
    add_definitions(-DINVIWO_ALL_DYN_LINK)
    add_definitions(-DGLM_SHARED_BUILD)
else(BUILD_SHARED_LIBS)
    add_definitions(-DFREEGLUT_STATIC)
    add_definitions(-DGLEW_STATIC)
endif(BUILD_SHARED_LIBS)

IF(WIN32)
    IF(MSVC)
        if(IVW_ENABLE_MSVC_MEMLEAK_TEST)
            add_definitions(-DIVW_ENABLE_MSVC_MEM_LEAK_TEST)
            if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
                link_directories(${IVW_EXTENSIONS_DIR}/vld/lib/Win64)
            else ()
                link_directories(${IVW_EXTENSIONS_DIR}/vld/lib/Win32)
            endif()
        endif(IVW_ENABLE_MSVC_MEMLEAK_TEST)    
    ENDIF(MSVC)
ENDIF(WIN32)

if(IVW_PROFILING)
    add_definitions(-DIVW_PROFILING)
endif(IVW_PROFILING)

IF(NOT MSVC)
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
    CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
    if(COMPILER_SUPPORTS_CXX11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    elseif(COMPILER_SUPPORTS_CXX0X)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    else()
        ivw_message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
    endif()
else()
    if( MSVC_VERSION LESS 1700 )       # VC10-/VS2010- 
        ivw_message(FATAL_ERROR "Inviwo requires C++11 features. " 
                        "You need at least Visual Studio 11 (Microsoft Visual Studio 2012), " 
                        "with Microsoft Visual C++ Compiler Nov 2012 CTP (v120_CTP_Nov2012).") 
    elseif( MSVC_VERSION EQUAL 1700 )  # VC11/VS2012 
        set(CMAKE_GENERATOR_TOOLSET "v120_CTP_Nov2012" CACHE STRING "Platform Toolset" FORCE) 
    endif() 
endif()



#--------------------------------------------------------------------
# Exclude stuff from cotire
set(IVW_COTIRE_EXCLUDES
    "${IVW_EXTENSIONS_DIR}/warn"
    "${IVW_MODULE_DIR}/unittests/ext"
  )

if(WIN32 AND MSVC)
    if(DEFINED MSVC_ACRO)
        list(APPEND IVW_COTIRE_EXCLUDES 
            "C:/Program Files (x86)/Microsoft Visual Studio ${MSVC_ACRO}.0/VC/include/thread"
            "C:/Program Files (x86)/Microsoft Visual Studio ${MSVC_ACRO}.0/VC/include/thr/xthread")
    endif()
endif()