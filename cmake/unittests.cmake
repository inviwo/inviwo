#################################################################################
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2013-2019 Inviwo Foundation
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

 ### Generate unittests for modules. ###

#--------------------------------------------------------------------
# Options for unittests
option(IVW_UNITTESTS "Enable unittests" ON)
option(IVW_UNITTESTS_RUN_ON_BUILD "Enable running unittest when building" ON)

#--------------------------------------------------------------------
# Add unittests
function(ivw_add_unittest)
    if(IVW_UNITTESTS)
        set(${CMAKE_PROJECT_NAME}_UNITTEST_FILES "${ARGN}" PARENT_SCOPE)
    endif()
endfunction()

#--------------------------------------------------------------------
# Inviwo Unittest Application
function(ivw_make_unittest_target name target)
    # Check if there are any tests
    if(NOT IVW_UNITTESTS OR NOT ${CMAKE_PROJECT_NAME}_UNITTEST_FILES)
        return()
    endif()

    set(test_name "inviwo-unittests-${name}")
    ivw_debug_message(STATUS "create unittests: ${name}")

    project(${test_name})
    #--------------------------------------------------------------------
    # Add source files
    set(SOURCE_FILES ${${CMAKE_PROJECT_NAME}_UNITTEST_FILES})
    ivw_group("Test Files" ${SOURCE_FILES})

    #--------------------------------------------------------------------
    # Create application
    add_executable(${test_name} ${SOURCE_FILES})
    target_link_libraries(${test_name} 
        PUBLIC 
        gtest
        gmock
        inviwo::testutil
        ${target}
    )
    set_target_properties(${test_name} PROPERTIES FOLDER unittests)

    if(WIN32 AND MSVC)
        target_link_libraries(${test_name} PRIVATE -DEBUG:FULL)
    endif()

    #--------------------------------------------------------------------
    # Define defintions and properties
    ivw_define_standard_definitions(${test_name} ${test_name})
    ivw_define_standard_properties(${test_name})

    ivw_memleak_setup(${test_name})

    #--------------------------------------------------------------------
    # Add run on build command
    if(IVW_UNITTESTS_RUN_ON_BUILD)
        add_custom_command(TARGET "${test_name}" POST_BUILD COMMAND "${test_name}" 
                           COMMENT "Running unittest for: ${test_name}" VERBATIM)
    endif()

endfunction()