 #################################################################################
 #
 # Inviwo - Interactive Visualization Workshop
 #
 # Copyright (c) 2013-2016 Inviwo Foundation
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
if(PYTHONLIBS_FOUND)
    add_subdirectory(${IVW_ROOT_DIR}/ext/pybind11)

    if(MSVC)
        # Prevent setting the /GL and -LTCG flag 
        set(PYBIND11_LTO_CXX_FLAGS "" CACHE INTERNAL "")
        set(PYBIND11_LTO_LINKER_FLAGS "" CACHE INTERNAL "")
    endif(MSVC)
endif(PYTHONLIBS_FOUND)


function (ivw_add_py_wrapper name)
    if(IVW_MODULE_PYTHON3)
        pybind11_add_module(${name} ${ARGN})
        set_target_properties(${name} PROPERTIES DEBUG_POSTFIX "")
        set_target_properties(${name} PROPERTIES PREFIX "")

        set_target_properties(${name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
        target_link_libraries(${name} PUBLIC ${${mod}_target})

        ivw_folder(${name} pybind11modules)
    endif()
endfunction()
