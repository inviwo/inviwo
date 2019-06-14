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

 ### Generate python binding targets for modules. ###

# Needed for the function pybind11_add_module and dependency pybind11::module 

set(_allPyBindWrappers "" CACHE INTERNAL  "")
if(PYTHONLIBS_FOUND)
    add_subdirectory(${IVW_EXTENSIONS_DIR}/pybind11)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        # Workaround
        CHECK_CXX_COMPILER_FLAG("-flto-partition=none" HAS_LTO_PARTITION_FLAG)
        if(HAS_LTO_PARTITION_FLAG)
            set(PYBIND11_LTO_CXX_FLAGS "" CACHE INTERNAL "")
            set(PYBIND11_LTO_LINKER_FLAGS "-flto-partition=one" CACHE INTERNAL "")
        endif(HAS_LTO_PARTITION_FLAG)
    endif()
endif(PYTHONLIBS_FOUND)

function (ivw_add_py_wrapper target)
    if(IVW_MODULE_PYTHON3)
        pybind11_add_module(${target} ${ARGN})
        set_target_properties(${target} PROPERTIES DEBUG_POSTFIX "")
        set_target_properties(${target} PROPERTIES PREFIX "")
        set_target_properties(${target} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
        ivw_define_standard_definitions(${target} ${target})
        ivw_folder(${target} pymodules)

        set(_allPyBindWrappers "${_allPyBindWrappers};${target}" CACHE INTERNAL  "_allPyBindWrappers")

        ivw_define_standard_properties(${target})

        # pybind will set the visibility to hidden by default, but we run into problems with dynamic cast
        # of our templated precision types on OSX if hidden is used. So until we figure out how to manage 
        # that make the visibility default.
        set_target_properties(${target} PROPERTIES CXX_VISIBILITY_PRESET "default")
    endif()
endfunction()
