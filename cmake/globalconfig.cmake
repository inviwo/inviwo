 #################################################################################
 #
 # Inviwo - Interactive Visualization Workshop
 #
 # Copyright (c) 2013-2017 Inviwo Foundation
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
 
set(IVW_MAJOR_VERSION 0)
set(IVW_MINOR_VERSION 9)
set(IVW_PATCH_VERSION 8)
set(IVW_VERSION ${IVW_MAJOR_VERSION}.${IVW_MINOR_VERSION}.${IVW_PATCH_VERSION})

#--------------------------------------------------------------------
# Requirement checks
if(MSVC) 
    if(MSVC_VERSION LESS 1900)
        message(FATAL_ERROR "Inviwo requires C++14 features. " 
                "You need at least Visual Studio 14 (Microsoft Visual Studio 2015) "
                "The latest Visual Studio version is available at "
                "https://www.visualstudio.com/en-us/downloads/download-visual-studio-vs.aspx")
    endif()
else()
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
    if(COMPILER_SUPPORTS_CXX14)
        set(CMAKE_CXX_STANDARD 14)
        set(CMAKE_CXX_STANDARD_REQUIRED ON)
        set(CMAKE_CXX_EXTENSIONS OFF)
    else()
        message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++14 support. "
                "Please use a different C++ compiler.")
    endif()
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(WARNING "Inviwo is only supported for 64-bit architectures.")
endif()


set_property(GLOBAL PROPERTY USE_FOLDERS On)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER cmake)

#--------------------------------------------------------------------
# CMake debugging
option(IVW_CMAKE_DEBUG "Print CMake Debug Information" OFF)

if(IVW_CMAKE_DEBUG)
    function(log_proj variable access value file stack)
        if(${access} STREQUAL "MODIFIED_ACCESS")
            get_filename_component(path ${file} DIRECTORY)
            get_filename_component(name ${path} NAME)
            message(STATUS "Variable: ${variable} = ${value}, ${name}")
        endif()
    endfunction()

    #variable_watch(PYTHON_EXECUTABLE)
    #variable_watch(_projectName log_proj)
endif()

#--------------------------------------------------------------------
# Only output error messages
function(message)
    if(IVW_CMAKE_DEBUG)
        _message(${ARGV})
    else()
        if( GET )
            list(GET ARGV 0 MessageType)
            if( MessageType STREQUAL FATAL_ERROR OR
                MessageType STREQUAL SEND_ERROR OR
                MessageType STREQUAL WARNING OR
                MessageType STREQUAL AUTHOR_WARNING)
                    list(REMOVE_AT ARGV 0)
                    _message(STATUS "${ARGV}")
            endif()
        endif()
    endif()
endfunction()

function(ivw_debug_message)
    if(IVW_CMAKE_DEBUG)
        _message(${ARGV})
    endif()
endfunction()

function(ivw_message)
    _message(${ARGV})
endfunction()


#--------------------------------------------------------------------
# Add own cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/modules")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_BINARY_DIR}/cmake/")

#--------------------------------------------------------------------
# Add globalmacros
include(${CMAKE_CURRENT_LIST_DIR}/globalutils.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/globalmacros.cmake)

#--------------------------------------------------------------------
# Add parameter for paths to external modules
set(IVW_EXTERNAL_MODULES "" CACHE STRING "Paths to directory containing external modules")

#--------------------------------------------------------------------
# Add parameter for paths to external projects
set(IVW_EXTERNAL_PROJECTS "" CACHE STRING "Paths to directory containing external projects")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib CACHE PATH
   "Single Directory for all static libraries.")

#--------------------------------------------------------------------
# Output paths for the executables, runtimes, archives and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin CACHE PATH
   "Single Directory for all Executables.")

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
endif()

if(NOT LIBRARY_OUTPUT_PATH)
    set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib CACHE PATH
        "Single output directory for building all libraries.")
endif()

mark_as_advanced(EXECUTABLE_OUTPUT_PATH LIBRARY_OUTPUT_PATH)

# Set Common Variables
get_filename_component(IVW_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR} PATH)
set(IVW_INCLUDE_DIR             ${IVW_ROOT_DIR}/include)
set(IVW_SOURCE_DIR              ${IVW_ROOT_DIR}/src)
set(IVW_CORE_INCLUDE_DIR        ${IVW_ROOT_DIR}/include/inviwo/core)
set(IVW_CORE_SOURCE_DIR         ${IVW_ROOT_DIR}/src/core)
set(IVW_QT_INCLUDE_DIR          ${IVW_ROOT_DIR}/include/inviwo/qt)
set(IVW_QT_SOURCE_DIR           ${IVW_ROOT_DIR}/src/qt)
set(IVW_APPLICATION_DIR         ${IVW_ROOT_DIR}/apps)
set(IVW_MODULE_DIR              ${IVW_ROOT_DIR}/modules)
set(IVW_RESOURCES_DIR           ${IVW_ROOT_DIR}/resources)
set(IVW_EXTENSIONS_DIR          ${IVW_ROOT_DIR}/ext)
set(IVW_BINARY_DIR              ${CMAKE_BINARY_DIR})
set(IVW_LIBRARY_DIR             ${LIBRARY_OUTPUT_PATH})
set(IVW_EXECUTABLE_DIR          ${EXECUTABLE_OUTPUT_PATH})
set(IVW_CMAKE_SOURCE_MODULE_DIR ${CMAKE_CURRENT_LIST_DIR}/modules)
set(IVW_CMAKE_BINARY_MODULE_DIR ${CMAKE_BINARY_DIR}/cmake)
set(IVW_CMAKE_TEMPLATES         ${IVW_ROOT_DIR}/cmake/templates)

#Generate headers
ivw_generate_module_paths_header()
configure_file(${IVW_CMAKE_TEMPLATES}/inviwocommondefines_template.h 
               ${CMAKE_BINARY_DIR}/modules/_generated/inviwocommondefines.h 
               @ONLY IMMEDIATE)

#--------------------------------------------------------------------
# Mac specific
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_definitions(-DDARWIN)
endif()

#--------------------------------------------------------------------
# Force use of GLFW context over QT context
option(IVW_USE_GLFW_NOT_OPENGLQT "Use GLFW for context creation instead of OpenGLQt module" OFF)
mark_as_advanced(FORCE IVW_USE_GLFW_NOT_OPENGLQT)

#--------------------------------------------------------------------
# Package creation
option(IVW_PACKAGE_PROJECT "Create Inviwo Package Project" OFF)

#--------------------------------------------------------------------
# Use and generate resources when available
include(${CMAKE_CURRENT_LIST_DIR}/compileresources.cmake)

#--------------------------------------------------------------------
# Calculate and display profiling information
option(IVW_PROFILING "Enable profiling" OFF)

#--------------------------------------------------------------------
# Build unittest for all modules
include(${CMAKE_CURRENT_LIST_DIR}/unittests.cmake)

#--------------------------------------------------------------------
# Use Visual Studio memory leak test
include(${CMAKE_CURRENT_LIST_DIR}/memleak.cmake)

#--------------------------------------------------------------------
# Build shared libs or static libs
mark_as_advanced(BUILD_SHARED_LIBS)
mark_as_advanced(FORCE GLM_DIR)
mark_as_advanced(FORCE CMAKE_CONFIGURATION_TYPES)

if(WIN32 AND MSVC)
    if(SHARED_LIBS)
        set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libs, else static libs" FORCE)
    else()
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libs, else static libs" FORCE)
    endif()
    
    # Determine runtime library linkage depending on SHARED_LIBS setting.
    # Shared runtime can be forced by setting the IVW_FORCE_SHARED_CRT option.
    option(IVW_FORCE_SHARED_CRT "Use shared runtime library linkage for Inviwo" OFF)
    mark_as_advanced(IVW_FORCE_SHARED_CRT)
    if(SHARED_LIBS OR IVW_FORCE_SHARED_CRT)
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MD")
        set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} /MD")
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MDd")
        set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /MD")

        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
        set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /MD")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MD")
    else()
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
        set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} /MT")
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
        set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /MT")

        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
        set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /MT")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT")
    endif()

    # For >=VS2015 enable edit and continue "ZI"
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /ZI")



    # enable debug:fastlink for debug builds
    # https://blogs.msdn.microsoft.com/vcblog/2014/11/12/speeding-up-the-incremental-developer-build-scenario/
    # needs to be off for proper callstack from VLD https://vld.codeplex.com/discussions/654355 
    if(NOT IVW_ENABLE_MSVC_MEMLEAK_TEST)
        set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG:FASTLINK")
        set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /DEBUG:FASTLINK")
    endif()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")


    # set iterator debug level (default=2)
    # https://msdn.microsoft.com/en-us/library/hh697468.aspx
    set(IVW_ITERATOR_DEBUG_LEVEL "2" CACHE STRING "Iterator debug level (IDL, default=2). 
    IDL=0: Disables checked iterators and disables iterator debugging.
    IDL=1: Enables checked iterators and disables iterator debugging.
    IDL=2: Enables iterator debugging. Note: QT needs to be built with the same flag")
    set_property(CACHE IVW_ITERATOR_DEBUG_LEVEL PROPERTY STRINGS 0 1 2)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D_ITERATOR_DEBUG_LEVEL=${IVW_ITERATOR_DEBUG_LEVEL}")

    # MSVC Variable checks and include redist in packs
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(CMAKE_MSVC_ARCH x64)
    else()
        set(CMAKE_MSVC_ARCH x86)
    endif()
    # default dll dependencies for Visual Studio versions 2005, 2008, 2010, and 2013
    # In VS 2015, "msvcr140.dll" was replaced by several other dlls.
    set(MSVC_DLLNAMES "msvcp" "msvcr")
    if(MSVC14)
        set(MSVC_ACRO "14")
        set(MSVC_DLLNAMES "msvcp" "concrt" "vccorlib" "vcruntime")
    endif()
    set(MSVC_REDIST_DIR ${MSVC${MSVC_ACRO}_REDIST_DIR})

    if(IVW_PACKAGE_PROJECT AND BUILD_SHARED_LIBS)
        if(DEFINED MSVC_ACRO)
            foreach(dllname ${MSVC_DLLNAMES})
                # debug build
                install(FILES "${MSVC_REDIST_DIR}/Debug_NonRedist/${CMAKE_MSVC_ARCH}/Microsoft.VC${MSVC_ACRO}0.DebugCRT/${dllname}${MSVC_ACRO}0d.dll" 
                        DESTINATION bin 
                        COMPONENT core 
                        CONFIGURATIONS Debug)
                # release build
                install(FILES "${MSVC_REDIST_DIR}/${CMAKE_MSVC_ARCH}/Microsoft.VC${MSVC_ACRO}0.CRT/${dllname}${MSVC_ACRO}0.dll" 
                        DESTINATION bin 
                        COMPONENT core 
                        CONFIGURATIONS Release)
            endforeach()
        endif()
    endif()

    # Multicore builds
    option(IVW_MULTI_PROCESSOR_BUILD "Build with multiple processors" ON)
    set(IVW_MULTI_PROCESSOR_COUNT 0 CACHE STRING "Number of cores to use (defalt 0 = all)")
    if(IVW_MULTI_PROCESSOR_BUILD)
        if(IVW_MULTI_PROCESSOR_COUNT GREATER 1 AND IVW_MULTI_PROCESSOR_COUNT LESS 1024)
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP${IVW_MULTI_PROCESSOR_COUNT}")
            SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP${IVW_MULTI_PROCESSOR_COUNT}")
        else()
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
            SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
        endif()
    endif()  
endif()

if(UNIX AND NOT APPLE)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON) # Will add -fPIC under linux.
    set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--as-needed") # Only link to libs as needed.
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
            if(WIN32 AND MSVC AND DEFINED MSVC_ACRO AND DEFINED MSVC_REDIST_DIR)
                install(FILES "${MSVC_REDIST_DIR}/Debug_NonRedist/${CMAKE_MSVC_ARCH}/Microsoft.VC${MSVC_ACRO}0.DebugOpenMP/vcomp${MSVC_ACRO}0d.dll" DESTINATION bin COMPONENT core CONFIGURATIONS Debug)
                install(FILES "${MSVC_REDIST_DIR}/${CMAKE_MSVC_ARCH}/Microsoft.VC${MSVC_ACRO}0.OPENMP/vcomp${MSVC_ACRO}0.dll" DESTINATION bin COMPONENT core CONFIGURATIONS Release)
            endif()
        endif()
    endif()
endif()

#--------------------------------------------------------------------
# Set preprocessor definition to indicate whether 
# to use the debug postfix
# Add debug postfix if WIN32
IF(WIN32 AND MSVC)
    SET(CMAKE_DEBUG_POSTFIX "d")
ENDIF()

if(DEBUG_POSTFIX)
    add_definitions(-D_DEBUG_POSTFIX)
endif(DEBUG_POSTFIX)

#--------------------------------------------------------------------
# Specify build-based defintions
if(BUILD_SHARED_LIBS)
    add_definitions(-DINVIWO_ALL_DYN_LINK)
    add_definitions(-DGLM_SHARED_BUILD)
else(BUILD_SHARED_LIBS)
    add_definitions(-DFREEGLUT_STATIC)
    add_definitions(-DGLEW_STATIC)
endif(BUILD_SHARED_LIBS)

if(IVW_PROFILING)
    add_definitions(-DIVW_PROFILING)
endif(IVW_PROFILING)


#--------------------------------------------------------------------
# Precompile headers
if(WIN32)
    option(PRECOMPILED_HEADERS "Create and use precompilied headers" ON)
else()
    option(PRECOMPILED_HEADERS "Create and use precompilied headers" OFF)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/utilities/clean_library_list.cmake)
if(PRECOMPILED_HEADERS)
    include(${CMAKE_CURRENT_LIST_DIR}/cotire.cmake)
endif()

mark_as_advanced(
    COTIRE_ADDITIONAL_PREFIX_HEADER_IGNORE_EXTENSIONS 
    COTIRE_ADDITIONAL_PREFIX_HEADER_IGNORE_PATH 
    COTIRE_DEBUG 
    COTIRE_MAXIMUM_NUMBER_OF_UNITY_INCLUDES 
    COTIRE_MINIMUM_NUMBER_OF_TARGET_SOURCES
    COTIRE_UNITY_SOURCE_EXCLUDE_EXTENSIONS
    COTIRE_VERBOSE
)

# Exclude stuff from cotire
set(IVW_COTIRE_EXCLUDES
    "${IVW_EXTENSIONS_DIR}/warn"
)


