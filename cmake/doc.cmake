#################################################################################
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2023-2025 Inviwo Foundation
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

define_property(
    GLOBAL PROPERTY IVW_DOC_INPUT_DIRS
    BRIEF_DOCS "List of doxygen input dirs"
)
define_property(
    GLOBAL PROPERTY IVW_DOC_IMAGE_PATHS
    BRIEF_DOCS "List of doxygen image paths"
)
define_property(
    GLOBAL PROPERTY IVW_DOC_INCLUDE_PATHS
    BRIEF_DOCS "List of doxygen include dirs"
)

function(ivw_get_include_dirs retval)
    set(inc_dirs "")
    foreach(target IN LISTS ARGN)
        ivw_get_target_property_recursive(dirs ${target} INTERFACE_INCLUDE_DIRECTORIES True)
        list(APPEND inc_dirs ${dirs})
        ivw_get_target_property_recursive(dirs ${target} INCLUDE_DIRECTORIES False)
        list(APPEND inc_dirs ${dirs})
        list(REMOVE_DUPLICATES inc_dirs)
    endforeach()
    
    # filter generator expr
    set(inc_dirs_nogen "")
    foreach(item IN LISTS inc_dirs)
        string(REGEX MATCH [=[^\$<BUILD_INTERFACE:([^$<>]+)>$]=] repl ${item})
        if(CMAKE_MATCH_1)
            list(APPEND inc_dirs_nogen ${CMAKE_MATCH_1})
        endif()
        string(REGEX MATCH [=[^[^$<>]*$]=] repl ${item})
        if(CMAKE_MATCH_0)
            list(APPEND inc_dirs_nogen ${CMAKE_MATCH_0})
        endif()
    endforeach()
    list(REMOVE_DUPLICATES inc_dirs_nogen)
    
    # filter non existing folders
    set(inc_dirs_exist "")
    foreach(item IN LISTS inc_dirs_nogen)
        if(EXISTS ${item})
            get_filename_component(real ${item} REALPATH)
            list(APPEND inc_dirs_exist ${real})
        endif()
    endforeach()
    list(REMOVE_DUPLICATES inc_dirs_exist)
    list(SORT inc_dirs_exist)

    set(${retval} ${inc_dirs_exist} PARENT_SCOPE)
endfunction()

function(ivw_register_docs)
    set(options )
    set(oneValueArgs )
    set(multiValueArgs INPUT_DIRS IMAGE_PATHS INCLUDE_PATHS INCLUDE_PATHS_FROM_TARGETS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(ARG_INPUT_DIRS)
        get_property(input_dirs GLOBAL PROPERTY IVW_DOC_INPUT_DIRS)
        list(APPEND input_dirs ${ARG_INPUT_DIRS})
        set_property(GLOBAL PROPERTY IVW_DOC_INPUT_DIRS ${input_dirs})
    endif()

    if(ARG_IMAGE_PATHS)
        get_property(image_paths GLOBAL PROPERTY IVW_DOC_IMAGE_PATHS)
        list(APPEND image_paths ${ARG_IMAGE_PATHS})
        set_property(GLOBAL PROPERTY IVW_DOC_IMAGE_PATHS ${image_paths})
    endif()

    if(ARG_INCLUDE_PATHS)
        get_property(include_paths GLOBAL PROPERTY IVW_DOC_INCLUDE_PATHS)
        list(APPEND include_paths ${ARG_INCLUDE_PATHS})
        set_property(GLOBAL PROPERTY IVW_DOC_INCLUDE_PATHS ${include_paths})
    endif()

    if(ARG_INCLUDE_PATHS_FROM_TARGETS)
        ivw_get_include_dirs(new_include_paths ${ARG_INCLUDE_PATHS_FROM_TARGETS})
        get_property(include_paths GLOBAL PROPERTY IVW_DOC_INCLUDE_PATHS)
        list(APPEND include_paths ${new_include_paths})
        set_property(GLOBAL PROPERTY IVW_DOC_INCLUDE_PATHS ${include_paths})
    endif()
endfunction()
