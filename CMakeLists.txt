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

cmake_minimum_required(VERSION 3.16...3.25 FATAL_ERROR)
string(TIMESTAMP timeStart "%Y-%m-%d %H:%M:%S")

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

if(DEFINED IVW_CFG_INCLUDE)
    include(${IVW_CFG_INCLUDE})
endif()

# option for setting the project/solution name of the Inviwo project
set(IVW_CFG_PROJECT_NAME "inviwo-projects" CACHE STRING "Project/Solution name (default: inviwo-projects)")
project(${IVW_CFG_PROJECT_NAME} 
    HOMEPAGE_URL https://github.com/inviwo/inviwo
    LANGUAGES CXX
)

#Verify that git submodules has been cloned
include(cmake/verifysubmodules.cmake)
verify_submodules("${CMAKE_CURRENT_LIST_DIR}/.gitmodules")

option(BUILD_SHARED_LIBS "Build shared libs, else static libs" ON)

set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)
file(REMOVE_RECURSE ${CMAKE_BINARY_DIR}/pkg)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake/modules)

find_package(Python3 COMPONENTS Interpreter Development NumPy)

include(cmake/globalconfig.cmake)

#--------------------------------------------------------------------
# Application setup
option(IVW_APP_QTBASE       "Build base for Qt apps. Used by the Qt network editor and the Tiny Qt app" ON)
option(IVW_APP_INVIWO       "Build Inviwo Qt network editor application" ON)
option(IVW_APP_MINIMAL_GLFW "Build Inviwo Tiny GLFW Application" OFF)
option(IVW_APP_MINIMAL_QT   "Build Inviwo Tiny QT Application" OFF)
option(IVW_APP_PYTHON       "Build Inviwo Python Application" ON)

if((IVW_APP_INVIWO OR IVW_APP_MINIMAL_QT OR IVW_APP_PYTHON) AND NOT IVW_APP_QTBASE)
    set(IVW_APP_QTBASE ON CACHE BOOL 
        "Build base for qt applications. Used by Qt network editor and Qt minimal application" 
        FORCE
    )
    message(STATUS "IVW_APP_QTBASE was set to build, due to dependencies")
endif()

ivw_enable_modules_if(IVW_APP_INVIWO QtWidgets)
ivw_enable_modules_if(IVW_APP_MINIMAL_QT QtWidgets)
ivw_enable_modules_if(IVW_APP_MINIMAL_GLFW GLFW)
ivw_enable_modules_if(IVW_APP_INVIWO_DOME SGCT)
ivw_enable_modules_if(IVW_APP_PYTHON Python3 Python3Qt QtWidgets)

option(IVW_TEST_INTEGRATION_TESTS "Build inviwo integration test" ON)
ivw_enable_modules_if(IVW_TEST_INTEGRATION_TESTS GLFW Base)

# Try to find qt and add it if it is not already in CMAKE_PREFIX_PATH
if(NOT "${CMAKE_PREFIX_PATH}" MATCHES "[Qq][Tt]")
    include(cmake/locateqt.cmake)
    ivw_locateqt(QT_PATH)
    if(QT_PATH)
        ivw_debug_message(STATUS "Found qt at: ${QT_PATH}")
        list(APPEND CMAKE_PREFIX_PATH ${QT5_PATH})
    endif()
endif()

add_custom_target(uninstall COMMENT "Dummy target to prevent other uninstalls")

add_subdirectory(ext)                        # Add external libraries.
add_subdirectory(tools/meta)                 # Add inviwo meta library, used in the qt editor
add_subdirectory(tools/tracy)                # Add tracy wrapper, used for profiling
if(IVW_TEST_INTEGRATION_TESTS OR IVW_TEST_UNIT_TESTS)
    add_subdirectory(tests/testutil)         # Add test utils, used in the module unit tests
endif()                    
ivw_register_modules(all_modules)            # Add inviwo modules
add_subdirectory(src/qt)                     # Add the Qt network editor
add_subdirectory(apps)                       # Add all the applications, uses the modules.
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
