 #################################################################################
 #
 # Inviwo - Interactive Visualization Workshop
 #
 # Copyright (c) 2012-2018 Inviwo Foundation
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

cmake_minimum_required(VERSION 3.12.0)
cmake_policy(VERSION 3.12.0)

if(POLICY CMP0077)
     cmake_policy(SET CMP0077 NEW)
endif()

string(TIMESTAMP timeStart "%Y-%m-%d %H:%M:%S")

#--------------------------------------------------------------------
# Disallow in-source builds, we require an out of source build
get_filename_component(srcdir "${CMAKE_SOURCE_DIR}" REALPATH)
get_filename_component(bindir "${CMAKE_BINARY_DIR}" REALPATH)
set(prevbindir "")
while(NOT "${bindir}" STREQUAL "${prevbindir}")
    if("${bindir}" STREQUAL "${srcdir}")
        message(FATAL_ERROR
            "In-source builds are not permitted!\n"
            "Make a separate folder for building, outside of the source directory.")
    endif()
    set(prevbindir "${bindir}")
    get_filename_component(bindir ${bindir} DIRECTORY)
endwhile()
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# option for setting the project/solution name of the Inviwo project
set(IVW_PROJECT_NAME "inviwo-projects" CACHE STRING "Project/Solution name (default: inviwo-projects)")
project(${IVW_PROJECT_NAME})

#Verify that git submodules has been cloned
include(cmake/verifysubmodules.cmake)
verify_submodules("${CMAKE_CURRENT_LIST_DIR}/.gitmodules")

option(BUILD_SHARED_LIBS "Build shared libs, else static libs" ON)

# Needed for find_package(PythonLibsNew 3), which is an improved version of CMake shipped find_pythonLibs
set(CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}/pkg/;${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/ext/pybind11/tools")

# Make sure we do look for python 3 here, before it is done in any subdirectory or external library.
# If we do not specify version number on our first call to find_package(PythonLibs) it returns the
# first python version that is found on the system and then set the PYTHON_FOUND variable, this
# version may be 2.x and not 3.x and later when we find_package(PythonLibsNew 3) (or find_package(PythonLibs 3))
# it will detected that PYTHON_FOUND is set and reuse the paths found before and not look for python
# again resulting in Python 2.x binaries will be used.
set(PythonLibsNew_FIND_VERSION 3)
set(Python_ADDITIONAL_VERSIONS 3.5 3.6 3.7 3.8 3.9)
if(IVW_MODULE_PYTHON3)
	find_package(PythonLibsNew 3 REQUIRED)
else()
	find_package(PythonLibsNew 3)
endif()

include(cmake/globalconfig.cmake)

#--------------------------------------------------------------------
# Application setup
option(IVW_QT_APPLICATION_BASE
    "Build base for Qt apps. Used by the Qt network editor and the Tiny Qt app" ON)
option(IVW_QT_APPLICATION        "Build Inviwo Qt network editor application" ON)
option(IVW_INTEGRATION_TESTS     "Build inviwo integration test" ON)
option(IVW_TINY_GLFW_APPLICATION "Build Inviwo Tiny GLFW Application" OFF)
option(IVW_TINY_QT_APPLICATION   "Build Inviwo Tiny QT Application" OFF)

if(IVW_QT_APPLICATION AND NOT IVW_QT_APPLICATION_BASE)
    set(IVW_QT_APPLICATION_BASE ON CACHE BOOL "Build base for qt applications. \
            Used by Qt network editor and Qt minimal application" FORCE)
    message(STATUS "IVW_QT_APPLICATION_BASE was set to build, due to dependency towards IVW_QT_APPLICATION")
endif()
ivw_enable_modules_if(IVW_QT_APPLICATION QtWidgets)
ivw_enable_modules_if(IVW_INTEGRATION_TESTS GLFW Base)
ivw_enable_modules_if(IVW_TINY_GLFW_APPLICATION GLFW)

# Try to find qt and add it if it is not already in CMAKE_PREFIX_PATH
if(NOT "${CMAKE_PREFIX_PATH}" MATCHES "[Qq][Tt]")
    include(cmake/locateqt.cmake)
    ivw_locateqt(QT5_PATH)
    if(QT5_PATH)
        ivw_debug_message(STATUS "Found qt at: ${QT5_PATH}")
        list(APPEND CMAKE_PREFIX_PATH ${QT5_PATH})
    endif()
endif()

add_custom_target(uninstall COMMENT "Dummy target to prevent other uninstalls")

#--------------------------------------------------------------------
# Add external libraries.
#
#
# Dependencies between libraries are solved using the given order
# Note: We prefer building dependencies our self to using find_library,
# since it does not necessarily give you the dll for packaging.
# It also ensures that we are in control of the used version
# and is built using the same, Inviwo, settings.

option (IVW_USE_EXTERNAL_IMG "Link with external image libraries (zlib, png, jpeg, tiff) instead of building them." OFF)
if (NOT IVW_USE_EXTERNAL_IMG)
  add_subdirectory(ext/zlib-1.2.11) # Defines target ZLIB::ZLIB
  add_subdirectory(ext/libpng)      # Defines target PNG::PNG
  add_subdirectory(ext/libjpeg)     # Defines target JPEG::JPEG
  add_subdirectory(ext/tiff)        # Defines target TIFF::TIFF
endif()

add_subdirectory(ext/flags)
add_subdirectory(ext/glm)
add_subdirectory(ext/ticpp)
add_subdirectory(ext/gtest)
add_subdirectory(ext/benchmark)
add_subdirectory(ext/warn)
add_subdirectory(ext/tclap)
add_subdirectory(ext/tinydir)
add_subdirectory(ext/half)
add_subdirectory(ext/utf)
add_subdirectory(ext/json)
add_subdirectory(ext/fmt)
add_subdirectory(ext/sml)
add_subdirectory(ext/sigar)
add_subdirectory(ext/stackwalker) # Add stackwalker for windows for stack traces in the log
add_subdirectory(tests/testutil)

ivw_register_modules(all_modules)        # Add modules

add_subdirectory(tools/meta)             # Add Inviwo meta
add_subdirectory(src/qt)                 # Add Qt
add_subdirectory(apps)                   # Add applications
ivw_add_external_projects()              # Add external projects
add_subdirectory(tests/integrationtests) # Add integration tests project
include(cmake/packaging.cmake)           # Package creation

# Generate Doxygen setup.
add_subdirectory(docs)

string(TIMESTAMP time "%Y-%m-%d %H:%M:%S")
message(STATUS "Configure started at ${timeStart} and ended at ${time}")
