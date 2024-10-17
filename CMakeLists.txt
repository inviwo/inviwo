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

cmake_minimum_required(VERSION 3.23...3.30 FATAL_ERROR)
string(TIMESTAMP timeStart "%Y-%m-%d %H:%M:%S")
message(STATUS "CMake version: ${CMAKE_VERSION}")

set(IVW_MAJOR_VERSION 0)
set(IVW_MINOR_VERSION 9)
set(IVW_PATCH_VERSION 12)
set(IVW_PRERELEASE_VERSION pre)
set(IVW_CFG_BUILD_VERSION "" CACHE STRING "Additional build info to add to the inviwo version")

set(IVW_VERSION "${IVW_MAJOR_VERSION}.${IVW_MINOR_VERSION}.${IVW_PATCH_VERSION}")
if(NOT IVW_PRERELEASE_VERSION STREQUAL "")
    string(APPEND IVW_VERSION "-${IVW_PRERELEASE_VERSION}")
endif()
if(NOT IVW_CFG_BUILD_VERSION STREQUAL "")
    string(APPEND IVW_VERSION "+${IVW_CFG_BUILD_VERSION}")
endif()

# option for setting the project/solution name of the Inviwo project
set(IVW_CFG_PROJECT_NAME "inviwo-projects" CACHE STRING "Project/Solution name (default: inviwo-projects)")
project(${IVW_CFG_PROJECT_NAME} 
    HOMEPAGE_URL https://github.com/inviwo/inviwo
    LANGUAGES CXX C
    VERSION ${IVW_MAJOR_VERSION}.${IVW_MINOR_VERSION}.${IVW_PATCH_VERSION}
)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

enable_testing()

option(IVW_CFG_USE_CCACHE_MSVC "build using cache on windows" OFF)
if(IVW_CFG_USE_CCACHE_MSVC)
    # By default Visual Studio generators will use /Zi which is not compatible
    # with ccache, so tell Visual Studio to use /Z7 instead.
    message(STATUS "Setting MSVC debug information format to 'Embedded'")
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>")

    # this expects a copy of ccache.exe to be in CMAKE_BINARY_DIR renamed to cl.exe
    set(CMAKE_VS_GLOBALS
        "CLToolExe=cl.exe"
         "CLToolPath=${CMAKE_BINARY_DIR}"
        "TrackFileAccess=false"
        "UseMultiToolTask=true"
        "DebugInformationFormat=OldStyle"
    )
endif()
option(IVW_CFG_USE_CCACHE_XCODE "build using cache in xcode" OFF)
if(IVW_CFG_USE_CCACHE_XCODE)
    message(STATUS "Configuring ccache for use with XCode")
    message(STATUS " * CXX = ${CMAKE_CXX_COMPILER}")
    message(STATUS " * C   = ${CMAKE_C_COMPILER}")

    file(CONFIGURE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/launcher-c   CONTENT [=[#!/bin/sh
        export CCACHE_SLOPPINESS=clang_index_store
        exec ccache @CMAKE_C_COMPILER@ "$@"
    ]=]
    @ONLY)
    file(CONFIGURE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/launcher-cxx CONTENT [=[#!/bin/sh
        export CCACHE_SLOPPINESS=clang_index_store
        exec ccache @CMAKE_CXX_COMPILER@ "$@"
    ]=]
    @ONLY)
    file(CHMOD ${CMAKE_CURRENT_BINARY_DIR}/launcher-c ${CMAKE_CURRENT_BINARY_DIR}/launcher-cxx
         FILE_PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE)

    set(CMAKE_XCODE_ATTRIBUTE_CC          "${CMAKE_CURRENT_BINARY_DIR}/launcher-c")
    set(CMAKE_XCODE_ATTRIBUTE_CXX         "${CMAKE_CURRENT_BINARY_DIR}/launcher-cxx")
    set(CMAKE_XCODE_ATTRIBUTE_LD          "${CMAKE_CURRENT_BINARY_DIR}/launcher-c")
    set(CMAKE_XCODE_ATTRIBUTE_LDPLUSPLUS  "${CMAKE_CURRENT_BINARY_DIR}/launcher-cxx")
endif()

option(BUILD_SHARED_LIBS "Build shared libs, else static libs" ON)

file(REMOVE_RECURSE ${CMAKE_BINARY_DIR}/pkg)

# Provide option to enable Python support in Inviwo
option(IVW_ENABLE_PYTHON "Enable Python support in Inviwo" ON)
if(IVW_ENABLE_PYTHON)
    find_package(Python3 COMPONENTS Interpreter Development NumPy)
    if(NOT Python3_FOUND OR 
       NOT Python3_Interpreter_FOUND OR 
       NOT Python3_Development_FOUND OR 
       NOT Python3_NumPy_FOUND)
        message(FATAL_ERROR "Python3 not available and/or NumPy module not installed.\n"
            "Please install Python3 and NumPy or disable IVW_ENABLE_PYTHON.\n"
            "If you have a python installation that is not found, consider "
            "setting Python3_ROOT_DIR to the root directory of your "
            "Python 3 installation.\n"
            "Note: Inviwo will not access user site-package folders. "
            "Make sure to install the packages site-wide or add "
            "your user site-package folder to the environment variable `PYTHONPATH`,\n"
            "for example \"PYTHONPATH=%appdata%\\Python\\Python311\\site-packages\"."
            )
    endif()

    message(STATUS "    executable:  ${Python3_EXECUTABLE}")
    message(STATUS "    sitelib:     ${Python3_SITELIB}")
    message(STATUS "    include dir: ${Python3_INCLUDE_DIRS}")
    message(STATUS "    numpy:       ${Python3_NumPy_INCLUDE_DIRS}")
endif()

option(IVW_ENABLE_QT "Enable Qt support in Inviwo" ON)
if(IVW_ENABLE_QT)
    # Find Qt will add paths to CMAKE_MODULE_PATH which we don't want.
    set(CMAKE_MODULE_PATH_TMP ${CMAKE_MODULE_PATH})
    find_package(Qt6 COMPONENTS Core)
    if(NOT Qt6Core_FOUND) 
        message(FATAL_ERROR
            "Qt6 not available. Please install Qt or disable IVW_ENABLE_QT.\n"
            "Consider adding Qt to the CMAKE_PREFIX_PATH,\n" 
            "i.e. start cmake with the option -DCMAKE_PREFIX_PATH=\"/path/to/qt\"")
    endif()
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH_TMP})
endif()

include(cmake/globalconfig.cmake)

# Application setup
option(IVW_APP_INVIWO       "Build Inviwo Qt network editor application" ON)
option(IVW_APP_MINIMAL_GLFW "Build Inviwo Tiny GLFW Application" OFF)
option(IVW_APP_MINIMAL_QT   "Build Inviwo Tiny QT Application" OFF)
option(IVW_APP_INVIWO_DOME  "Build Inviwo Dome Application" OFF)
option(IVW_APP_PYTHON       "Build Inviwo Python Application" ON)

ivw_enable_modules_if(IVW_APP_INVIWO QtWidgets)
ivw_enable_modules_if(IVW_APP_MINIMAL_QT QtWidgets)
ivw_enable_modules_if(IVW_APP_MINIMAL_GLFW GLFW)
ivw_enable_modules_if(IVW_APP_INVIWO_DOME SGCT)
ivw_enable_modules_if(IVW_APP_PYTHON Python3 Python3Qt QtWidgets)

option(IVW_TEST_INTEGRATION_TESTS "Build inviwo integration test" ON)
ivw_enable_modules_if(IVW_TEST_INTEGRATION_TESTS GLFW Base)

add_custom_target(uninstall COMMENT "Dummy target to prevent other uninstalls")

add_subdirectory(ext)                        # Add external libraries.
add_subdirectory(tools/meta)                 # Add inviwo meta library, used in the qt editor
add_subdirectory(tools/tracy)                # Add tracy wrapper, used for profiling
if(IVW_TEST_INTEGRATION_TESTS OR IVW_TEST_UNIT_TESTS)
    add_subdirectory(tests/testutil)         # Add test utils, used in the module unit tests
endif()

add_subdirectory(src/sys)                    # Add inviwo module system
add_subdirectory(src/py)                     # Add inviwo python helper

# Add all the applications, uses the modules.
if(IVW_ENABLE_QT)
    add_subdirectory(src/qt/applicationbase)
    if(IVW_APP_INVIWO)
        add_subdirectory(src/qt/editor)
        add_subdirectory(apps/inviwo)
    endif()
    if(IVW_APP_PYTHON)
        add_subdirectory(apps/inviwopyapp)
    endif()
    if(IVW_APP_MINIMAL_QT)
        add_subdirectory(apps/inviwo_qtminimum)
    endif()
else()
    message(STATUS "QT support is not enabled, needed for IVW_APP_INVIWO, IVW_APP_PYTHON, IVW_APP_MINIMAL_QT\n"
            "Enable IVW_ENABLE_QT to use")
endif()
if(IVW_APP_MINIMAL_GLFW)
    add_subdirectory(apps/inviwo_glfwminimum)
endif()
if(IVW_APP_INVIWO_DOME)
    add_subdirectory(apps/inviwodome)
endif()

ivw_add_external_projects()                  # Add external projects
if(IVW_TEST_INTEGRATION_TESTS)
    add_subdirectory(tests/integrationtests) # Add integration tests, uses the modules.
endif()
add_subdirectory(docs)                       # Generate Doxygen targets

if(MSVC AND TARGET inviwo)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT inviwo)
endif()

string(TIMESTAMP time "%Y-%m-%d %H:%M:%S")
message(STATUS "Configure started at ${timeStart} and ended at ${time}")
