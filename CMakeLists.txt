 #################################################################################
 #
 # Inviwo - Interactive Visualization Workshop
 #
 # Copyright (c) 2012-2017 Inviwo Foundation
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
 
cmake_minimum_required(VERSION 3.2.0)
cmake_policy(VERSION 3.2.0)

if(POLICY CMP0020)
    cmake_policy(SET CMP0020 OLD) # Missing dependencies policy.
endif()

if(POLICY CMP0028)
    cmake_policy(SET CMP0028 OLD) # allow :: in targets. like Qt5::Sql
endif()

if(POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW) # MacOSX rpath policy.
endif()

if(POLICY CMP0043)
    # The OLD behavior for this policy is to consume the content of the 
    # suffixed COMPILE_DEFINITIONS_<CONFIG> target property when generating
    # the compilation command.
    cmake_policy(SET CMP0043 OLD)
endif()

if(POLICY CMP0046)
    cmake_policy(SET CMP0046 OLD) # Missing dependencies policy.
endif()

if(POLICY CMP0051)
    # include TARGET_OBJECTS expressions in a target's SOURCES property
    cmake_policy(SET CMP0051 NEW)
endif()

if(POLICY CMP0054)
    # only interpret if() arguments as variables or keywords when unquoted
    cmake_policy(SET CMP0054 NEW)
endif()

# option for setting the project/solution name of the Inviwo project
set(IVW_PROJECT_NAME "inviwo-projects" CACHE STRING "Project/Solution name (default: inviwo-projects)")

if(IVW_PROJECT_NAME)
    project(${IVW_PROJECT_NAME})
else()
    project(inviwo-projects)
endif()
# Build
option(SHARED_LIBS "Build shared libs, else static libs" ON)


# Needed for find_package(PythonLibsNew 3), which is an improved version of CMake shipped find_pythonLibs
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/ext/pybind11/tools") 


# Make sure we do look for python 3 here, before it is done in any subdirectory or external library. 
# If we do not specify version number on our first call to find_package(PythonLibs) it returns the 
# first python version that is found on the system and then set the PYTHON_FOUND variable, this 
# version may be 2.x and not 3.x and later when we find_package(PythonLibsNew 3) (or find_package(PythonLibs 3))
# it will detected that PYTHON_FOUND is set and reuse the paths found before and not look for python
# again resulting in Python 2.x binaries will be used. 
set(PythonLibsNew_FIND_VERSION 3)
set(Python_ADDITIONAL_VERSIONS 3.5 3.6 3.7 3.8 3.9)
find_package(PythonLibsNew 3)


include(cmake/globalconfig.cmake)

#--------------------------------------------------------------------
# Applications/Qt
option(IVW_QT_APPLICATION "Build Inviwo Qt network editor application" ON)
option(IVW_QT_APPLICATION_BASE "Build base for Qt applications. Used by Inviwo Qt network editor and Qt minimal application" ON)

string(FIND "${CMAKE_PREFIX_PATH}" qt hasqt) # Skip if qt is already in CMAKE_PREFIX_PATH
if(NOT ${hasqt} GREATER 0) # try and find qt and add it to CMAKE_PREFIX_PATH
    if(NOT QT_QMAKE_EXECUTABLE)
        find_program(QT_QMAKE_EXECUTABLE_FINDQT NAMES qmake qmake4 qmake-qt4 qmake5 qmake-qt5
           PATHS "${QT_SEARCH_PATH}/bin" "$ENV{QTDIR}/bin")
        set(QT_QMAKE_EXECUTABLE ${QT_QMAKE_EXECUTABLE_FINDQT} CACHE PATH "Qt qmake program.")
    endif()

    if(WIN32)
        if(QT_QMAKE_EXECUTABLE)
            string(REGEX REPLACE "bin\\\\qmake([^ ]*)$" "lib\\\\cmake" QT5_PATH "${QT_QMAKE_EXECUTABLE}")
        endif()
    elseif(APPLE)
        if(EXISTS "/usr/local/Cellar/qt")
            file(GLOB qtversions RELATIVE "/usr/local/Cellar/qt/" "/usr/local/Cellar/qt/?.?.*")
            list(LENGTH qtversions len)
            if(${len} GREATER 0)
                list(GET qtversions -1 qtlatest)
                if(EXISTS "/usr/local/Cellar/qt/${qtlatest}")   
                    set(QT5_PATH "/usr/local/Cellar/qt/${qtlatest}")          
                endif()
             endif()
        elseif(EXISTS "/usr/local/Cellar/qt5")
            file(GLOB qtversions RELATIVE "/usr/local/Cellar/qt5/" "/usr/local/Cellar/qt5/?.?.*")
            list(LENGTH qtversions len)
            if(${len} GREATER 0)
                list(GET qtversions -1 qtlatest)
                if(EXISTS "/usr/local/Cellar/qt5/${qtlatest}")   
                    set(QT5_PATH "/usr/local/Cellar/qt5/${qtlatest}")          
                endif()
             endif()
        endif()
    else()
        if(QT_QMAKE_EXECUTABLE)
            string(REGEX REPLACE "bin/qmake([^ ]*)$" "lib/cmake" QT5_PATH "${QT_QMAKE_EXECUTABLE}")
        endif()
    endif()

    if(QT5_PATH)
        ivw_debug_message(STATUS "Found qt at: ${QT5_PATH}") 
        list(APPEND CMAKE_PREFIX_PATH ${QT5_PATH}) 
    endif()
endif()

#--------------------------------------------------------------------
# Add external libraries.
# 
# Dependencies between libraries are solved using the given order
add_subdirectory(ext/zlib-1.2.11)
add_subdirectory(ext/libpng)
add_subdirectory(ext/libjpeg)
add_subdirectory(ext/openexr)
add_subdirectory(ext/flags)
add_subdirectory(ext/glm)
add_subdirectory(ext/ticpp)
add_subdirectory(ext/gtest)
add_subdirectory(ext/warn)
add_subdirectory(ext/tclap)
add_subdirectory(ext/tinydir)
add_subdirectory(ext/half)
add_subdirectory(ext/utf)

option(IVW_SIGAR "Use sigar to detect system capabilities" ON)
if(IVW_SIGAR)
    add_subdirectory(ext/sigar)
endif()

#--------------------------------------------------------------------
# Add stackwalker for windows for stack traces in the log
if(WIN32)
    add_subdirectory(ext/stackwalker)
endif()

#--------------------------------------------------------------------
# Add Core
add_subdirectory(src/core)

#--------------------------------------------------------------------
# Add default modules
set(ivw_default_modules
    "Assimp" 
    "Base"
    "BaseGL"
    "CImg"
    "FontRendering"
    "OpenGL"
    "OpenGLQt"
    "PVM"
    "Nifti"
    "QtWidgets" 
    "VectorFieldVisualization"
    "VectorFieldVisualizationGL"
    "PostProcessing"
    "Nifti" 
    CACHE INTERNAL "Default Inviwo Modules"
)


if(PYTHONLIBS_FOUND)
    set(ivw_default_modules
        ${ivw_default_modules}
        "Python3"
        "Python3Qt"
        )
endif()

find_package(OpenCL QUIET)
if(OPENCL_FOUND)
    set(ivw_default_modules
        ${ivw_default_modules}
        "OpenCL"
        "BaseCL"
        )
endif()

option(IVW_INTEGRATION_TESTS "Build inviwo integration test" ON)
ivw_add_build_module_dependency(GLFW IVW_INTEGRATION_TESTS)
ivw_add_build_module_dependency(BASE IVW_INTEGRATION_TESTS)

#--------------------------------------------------------------------
# Add modules
ivw_register_modules(all_modules)

# OpenGL Context created with GLFW or Qt within Inviwo by default
if(IVW_USE_GLFW_NOT_OPENGLQT)
    ivw_enable_module(GLFW)
else()
    ivw_enable_module(OpenGLQt)
endif()

#--------------------------------------------------------------------
# Add Qt
add_subdirectory(src/qt)

#--------------------------------------------------------------------
# Add applications
add_subdirectory(apps)
if(WIN32)
    if(NOT ${CMAKE_VERSION} VERSION_LESS "3.6")
        set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT inviwo-cli)
    endif()
endif()


#--------------------------------------------------------------------
# Add external projects
ivw_add_external_projects()


#--------------------------------------------------------------------
# Add integration tests project
if(IVW_INTEGRATION_TESTS)
    add_subdirectory(tests/integrationtests)
endif()

#--------------------------------------------------------------------
# Package creation
include(cmake/packaging.cmake)

#--------------------------------------------------------------------
# Generate Doxygen setup. 
include(tools/doxygen/doxygen.cmake)
make_doxygen_target(all_modules)
