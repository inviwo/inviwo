#################################################################################
# 
# Inviwo - Interactive Visualization Workshop
# 
# Copyright (c) 2013-2023 Inviwo Foundation
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

# Requirement checks
include(CheckCXXCompilerFlag)
if(MSVC) 
    # https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B
    if(MSVC_VERSION LESS 1930) # Visual Studio 2022 version 17.0.1
        message(FATAL_ERROR "Inviwo requires C++20 features. " 
                "You need at least Visual Studio 19 (Microsoft Visual Studio 2022) "
                "The latest Visual Studio version is available at "
                "https://www.visualstudio.com/en-us/downloads/download-visual-studio-vs.aspx")
    endif()
    CHECK_CXX_COMPILER_FLAG("/std:c++20" compiler_supports_cxx20)
else()
    CHECK_CXX_COMPILER_FLAG("-std=c++20" compiler_supports_cxx20)
endif()
if(NOT compiler_supports_cxx20)
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++20 support. "
            "Please use a different C++ compiler.")
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Inviwo is only supported for 64-bit architectures. Resolve the error by deleting "
	        "the cache (File->Delete Cache) and selecting 64-bit architecture when configuring.")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS On)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER cmake)

#--------------------------------------------------------------------
# CMake debugging
option(IVW_CFG_CMAKE_DEBUG "Print CMake Debug Information" OFF)

if(IVW_CFG_CMAKE_DEBUG)
    function(log_proj variable access value file stack)
        if(${access} STREQUAL "MODIFIED_ACCESS")
            get_filename_component(path ${file} DIRECTORY)
            get_filename_component(name ${path} NAME)
            message(STATUS "Variable: ${variable} = ${value}, ${name}")
        endif()
    endfunction()

    #variable_watch(OpenMP_ON)
    #variable_watch(_projectName log_proj)
endif()

# Make sure we print deprecation warnings
set(CMAKE_WARN_DEPRECATED ON)

function(ivw_debug_message)
    if(IVW_CFG_CMAKE_DEBUG)
        message(${ARGV})
    endif()
endfunction()

# Add parameter for paths to external modules
set(IVW_EXTERNAL_MODULES "" CACHE STRING "Semicolon (;) separated paths to directories containing external modules")
# Convert to valid paths, i.e. exchange backslash to slash
file(TO_CMAKE_PATH "${IVW_EXTERNAL_MODULES}" IVW_EXTERNAL_MODULES)
# Add parameter for paths to external projects
set(IVW_EXTERNAL_PROJECTS "" CACHE STRING "Semicolon (;) separated paths to directories with apps. CMake add_subdirectory will be called for each path.")
# Convert to valid paths, i.e. exchange backslash to slash
file(TO_CMAKE_PATH "${IVW_EXTERNAL_PROJECTS}" IVW_EXTERNAL_PROJECTS)

# Output paths for the executables, runtimes, archives and libraries
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib CACHE PATH
   "Single Directory for all static libraries.")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin CACHE PATH
   "Single Directory for all Executables.")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib CACHE PATH
   "Single Directory for all Libraries")
if(NOT EXECUTABLE_OUTPUT_PATH)
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE PATH 
        "Single output directory for building all executables.")
endif()
if(NOT LIBRARY_OUTPUT_PATH)
    set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib CACHE PATH
        "Single output directory for building all libraries.")
endif()

# Sets IVW_GENERATOR_IS_MULTI_CONFIG to true if the generator is multi config, 
# e.g. when building Debug or Release mode is selected in the IDE (like in Visual Studio) 
# and not through cmake (as done when using make or ninja)
get_property(IVW_GENERATOR_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

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
set(IVW_TOOLS_DIR               ${IVW_ROOT_DIR}/tools)
set(IVW_BINARY_DIR              ${CMAKE_BINARY_DIR})
set(IVW_LIBRARY_DIR             ${LIBRARY_OUTPUT_PATH})
set(IVW_EXECUTABLE_DIR          ${EXECUTABLE_OUTPUT_PATH})
set(IVW_CMAKE_SOURCE_MODULE_DIR ${CMAKE_CURRENT_LIST_DIR}/modules)
set(IVW_CMAKE_BINARY_MODULE_DIR ${CMAKE_BINARY_DIR}/cmake)
set(IVW_CMAKE_TEMPLATES         ${IVW_ROOT_DIR}/cmake/templates)

# Add globalmacros
include(${CMAKE_CURRENT_LIST_DIR}/precompileheaders.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/globalutils.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/compileoptions.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/installutils.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/deprecated.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/filegeneration.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/inviwocoredata.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/inviwomoduledata.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/inviworegistermodules.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/globalmacros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/inviwocreatemodule.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/licenses.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/vcpkghelpers.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/benchmark.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/doc.cmake)


#Generate headers
ivw_generate_module_paths_header()
configure_file(${IVW_CMAKE_TEMPLATES}/inviwocommondefines_template.h 
               ${CMAKE_BINARY_DIR}/modules/core/include/inviwo/core/inviwocommondefines.h 
               @ONLY IMMEDIATE)

# Use and generate resources when available
include(${CMAKE_CURRENT_LIST_DIR}/compileresources.cmake)

# Calculate and display profiling information
option(IVW_CFG_PROFILING "Enable profiling" OFF)

# Build unittest for all modules
include(${CMAKE_CURRENT_LIST_DIR}/unittests.cmake)

# Use pybind11 for python bindings
include(${CMAKE_CURRENT_LIST_DIR}/pybind11.cmake)

if(WIN32 AND MSVC)
    # Determine runtime library linkage depending on BUILD_SHARED_LIBS setting.
    # Shared runtime can be forced by setting the IVW_CFG_MSVC_FORCE_SHARED_CRT option.
    option(IVW_CFG_MSVC_FORCE_SHARED_CRT "Use shared runtime library linkage for Inviwo" OFF)
    mark_as_advanced(IVW_CFG_MSVC_FORCE_SHARED_CRT)
    if(BUILD_SHARED_LIBS OR IVW_CFG_MSVC_FORCE_SHARED_CRT)
        add_compile_options(
            $<$<CONFIG:Release>:/MD> 
            $<$<CONFIG:MinSizeRel>:/MD> 
            $<$<CONFIG:Debug>:/MDd> 
            $<$<CONFIG:RelWithDebInfo>:/MD>
        )
    else()
        add_compile_options(
            $<$<CONFIG:Release>:/MT> 
            $<$<CONFIG:MinSizeRel>:/MT> 
            $<$<CONFIG:Debug>:/MTd> 
            $<$<CONFIG:RelWithDebInfo>:/MT>
        )
    endif()

    add_compile_options(/bigobj)
    
    # Add debug postfix if WIN32
    SET(CMAKE_DEBUG_POSTFIX "d")

    # Multicore builds
    option(IVW_CFG_MSVC_MULTI_PROCESSOR_BUILD "Build with multiple processors" ON)
    set(IVW_CFG_MSVC_MULTI_PROCESSOR_COUNT 0 CACHE STRING "Number of cores to use (defalt 0 = all)")
    if(IVW_CFG_MSVC_MULTI_PROCESSOR_BUILD)
        if(IVW_CFG_MSVC_MULTI_PROCESSOR_COUNT GREATER 1 AND IVW_CFG_MSVC_MULTI_PROCESSOR_COUNT LESS 1024)
            add_compile_options(/MP${IVW_CFG_MSVC_MULTI_PROCESSOR_COUNT})
        else()
            add_compile_options(/MP)
        endif()
    endif()
endif()

# Mac specific
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_definitions(-DDARWIN)

    # Set RPATH (relative path) differently depending on build-time and install/package.
    # At build time we use full paths to the generatated libraries.
    # When packaging, however, we must use relative paths so that the app 
    # can be used on other computers.
    # See:
    # https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/RPATH-handling#always-full-rpath
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    # Use full RPATH for the build tree 
    set(CMAKE_SKIP_BUILD_RPATH FALSE)

    # Don't use the install RPATH when building 
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

    # loader_path is where the executable/library is located.
    # Frameworks are stored in Contents/Frameworks while the executable is in
    # Contents/MacOS
    list(APPEND CMAKE_INSTALL_RPATH "@loader_path/" "@loader_path/../Frameworks")

    # Add the automatically determined parts of the RPATH,
    # which point to directories outside the build tree, to the install RPATH
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

if(UNIX AND NOT APPLE)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON) # Will add -fPIC under linux.
    set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--as-needed") # Only link to libs as needed.
endif()

# Runtime module loading
option(IVW_CFG_RUNTIME_MODULE_LOADING 
       "Load modules from dynamic libraries (dll/so) at application startup" OFF)

# Check if OpenMP is available and set it to use, and include the dll in packs, except for MSVC
find_package(OpenMP QUIET)
if(MSVC)
    option(IVW_ENABLE_OPENMP "Use OpenMP" OFF)
else()
    option(IVW_ENABLE_OPENMP "Use OpenMP" ${OpenMP_CXX_FOUND})
endif()

if(IVW_ENABLE_OPENMP AND NOT OpenMP_CXX_FOUND)
    message(FATAL_ERROR "OpenMP not available, Uncheck IVW_ENABLE_OPENMP to disable using OpenMP")
endif()
