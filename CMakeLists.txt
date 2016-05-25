 #################################################################################
 #
 # Inviwo - Interactive Visualization Workshop
 #
 # Copyright (c) 2012-2015 Inviwo Foundation
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
 
cmake_minimum_required(VERSION 2.8.11)
cmake_policy(VERSION 2.8.11)

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

project(inviwo-projects)

# Build
option(SHARED_LIBS "Build shared libs, else static libs" ON)

include(cmake/globalconfig.cmake)

#--------------------------------------------------------------------
# Applications/Qt
option(IVW_QT_APPLICATION "Build Inviwo Qt Application" ON)
option(IVW_QT_EDITOR "Build Inviwo Qt Editor" ON)
option(IVW_QT_WIDGETS "Build Inviwo Qt Widgets" ON)

#--------------------------------------------------------------------
# Add externals
add_subdirectory(ext/ticpp)
add_subdirectory(ext/gtest)

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
# Add Qt
add_subdirectory(src/qt)

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
    "VectorFieldVisualization"
    "VectorFieldVisualizationGL"
	"Nifti" 
    CACHE INTERNAL "Default Inviwo Modules"
)

set(Python_ADDITIONAL_VERSIONS 3.5)
find_package(PythonLibs 3 QUIET)
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
# Add applications
add_subdirectory(apps)

#--------------------------------------------------------------------
# Add external projects
ivw_add_external_projects()

#--------------------------------------------------------------------
# Package creation
include(cmake/packaging.cmake)

#--------------------------------------------------------------------
# Generate Doxygen setup. 
include(tools/doxygen/doxygen.cmake)
make_doxygen_target(all_modules)

