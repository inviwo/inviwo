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
include(CMakeParseArguments)

function(ivw_private_filter_dependency_list retval module)
    set(the_list "")
    if(ARGN)
        foreach(item ${ARGN})
            string(REGEX MATCH "(^Inviwo.*.Module$)" found_item ${item})
            if(found_item)
                list(APPEND the_list ${item})
            else()
                string(TOLOWER ${module} l_module)
                message(WARNING "Found dependency: \"${item}\", "
                    "which is not an Inviwo module in depends.cmake for module: \"${module}\". "
                    "Incorporate non Inviwo module dependencies using regular target_link_libraries. "
                    "For example: target_link_libraries(inviwo-module-${l_module} PUBLIC ${item})")
            endif()
        endforeach()
    endif()
    set(${retval} ${the_list} PARENT_SCOPE)
endfunction()

function(ivw_private_check_dependency_list retval modules_var module)
    set(the_list "")
    if(ARGN)
        foreach(item ${ARGN})
            string(TOUPPER ${item} u_item)
            list(FIND ${modules_var} ${u_item} found)
            if(NOT ${found} EQUAL -1)
                list(APPEND the_list ${item})
            else()
                message(WARNING "Found dependency: \"${item}\", in depends.cmake for"
                    " module: \"${module}\". But no such Inviwo module was registered.")
            endif()
        endforeach()
    endif()
    set(${retval} ${the_list} PARENT_SCOPE)
endfunction()

# Set module to build by default if value is true
# Example ivw_enable_modules_if(IVW_INTEGRATION_TESTS GLFW Base)
# Needs to be called before ivw_register_modules
function(ivw_enable_modules_if value)
    if(NOT ${value}) 
        return()
    endif()
    foreach(dir IN LISTS ARGN)
        ivw_dir_to_mod_dep(mod ${dir})
        ivw_dir_to_mod_prefix(opt ${dir})
        if(DEFINED ${mod}_opt)
            if(NOT ${${mod}_opt})
                ivw_add_module_option_to_cache(${mod} ON FORCE)
                message(STATUS "${dir} was set to on, due to dependency from ${value}")
            endif()
        elseif(DEFINED ${opt})
             set(${opt} ON CACHE BOOL "Build inviwo module ${dir}" FORCE)
             message(STATUS "${dir} was set to on, due to dependency from ${value}")
        elseif(NOT ${mod}_enableExternal)
            set("${mod}_enableExternal" ON CACHE INTERNAL "Enable module for external dependency" FORCE)
            message(STATUS "${dir} was set to on, due to dependency from ${value}")
        endif()
    endforeach()
endfunction()

# Set module build option to true
function(ivw_enable_module the_module)
    ivw_dir_to_mod_dep(mod ${the_module})
    ivw_add_module_option_to_cache(${mod} ON)
endfunction()

# Creates source group structure recursively
function(ivw_group group_name)
    set(options "")
    set(oneValueArgs "BASE")
    set(multiValueArgs "")
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    ivw_dir_to_mod_dep(mod ${PROJECT_NAME})
    
    if(ARG_BASE AND IS_ABSOLUTE ${ARG_BASE})
        set(base ${ARG_BASE})
    elseif(ARG_BASE AND NOT IS_ABSOLUTE ${ARG_BASE})
         set(base ${CMAKE_CURRENT_SOURCE_DIR}/${ARG_BASE})
    elseif(group_name STREQUAL "Header Files" AND ${mod}_incPath)
        set(base "${${mod}_incPath}")
    elseif(group_name STREQUAL "Header Files" AND  EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include")
        set(base "${CMAKE_CURRENT_SOURCE_DIR}/include")
    elseif(group_name STREQUAL "Source Files" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src")
        set(base "${CMAKE_CURRENT_SOURCE_DIR}/src")
    elseif(group_name STREQUAL "Test Files" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests")
        set(base "${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests")
     elseif(group_name STREQUAL "Shader Files" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/glsl")
        set(base "${CMAKE_CURRENT_SOURCE_DIR}/glsl")
    else()
        set(base "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()

    source_group(TREE ${base} PREFIX ${group_name} FILES ${ARG_UNPARSED_ARGUMENTS})
endfunction()

# Add all external projects specified in cmake string IVW_EXTERNAL_PROJECTS
function(ivw_add_external_projects)
    foreach(project_root_path ${IVW_EXTERNAL_PROJECTS})
        string(STRIP ${project_root_path} project_root_path)
        get_filename_component(FOLDER_NAME ${project_root_path} NAME)
        add_subdirectory(${project_root_path} ${CMAKE_CURRENT_BINARY_DIR}/ext_${FOLDER_NAME})
    endforeach()
endfunction()