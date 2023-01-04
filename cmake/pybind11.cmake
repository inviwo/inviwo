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

 ### Generate python binding targets for modules. ###

# Needed for the function pybind11_add_module and dependency pybind11::module 

define_property(
    GLOBAL PROPERTY IVW_PYMODULE_LIST
    BRIEF_DOCS "List of inviwo python module targets"
    FULL_DOCS "List of inviwo python module targets"
)

define_property(
    GLOBAL PROPERTY IVW_PYAPP_LIST
    BRIEF_DOCS "List of inviwo python app targets"
    FULL_DOCS "List of inviwo python app targets"
)

function(ivw_add_py_wrapper target)
    if(IVW_MODULE_PYTHON3)
        find_package(pybind11 CONFIG REQUIRED)

        if(BUILD_SHARED_LIBS) 
            pybind11_add_module(${target} NO_EXTRAS ${ARGN})
            set_target_properties(${target} PROPERTIES 
                DEBUG_POSTFIX ""
                LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
            
                # pybind will set the visibility to hidden by default, but we run into 
                # problems with dynamic cast of our templated precision types on OSX 
                # if hidden is used. So until we figure out how to manage that make 
                # the visibility default.
                CXX_VISIBILITY_PRESET "default"
            )
        else()
            add_library(${target} STATIC ${ARGN})
            target_link_libraries(${target} PUBLIC 
                pybind11::pybind11
                pybind11::embed
            )
        endif()
        ivw_define_standard_definitions(${target} ${target})
        ivw_define_standard_properties(${target})
        ivw_folder(${target} pymodules)

        target_link_libraries(${target} PRIVATE 
            $<$<CXX_COMPILER_ID:MSVC>:pybind11::windows_extras>
            pybind11::lto
        )

        set_property(GLOBAL APPEND PROPERTY IVW_PYMODULE_LIST ${target})
    endif()
endfunction()

function(ivw_check_python_module module retval)
    find_package(Python3 COMPONENTS Interpreter QUIET)

    if (NOT Python3_Interpreter_FOUND)
        set(${retval} FALSE PARENT_SCOPE)
        return()
    endif()

    execute_process(COMMAND "${Python3_EXECUTABLE}" "-c" 
        "import sys\ntry:\n\timport ${module}\nexcept ImportError:\n\tsys.exit(1)"
        RESULT_VARIABLE result 
        ERROR_QUIET)
    if(NOT result EQUAL 0)
        set(${retval} FALSE PARENT_SCOPE)
    else()
        set(${retval} TRUE PARENT_SCOPE)
    endif()
endfunction()
