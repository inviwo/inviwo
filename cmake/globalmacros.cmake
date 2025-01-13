#################################################################################
# 
# Inviwo - Interactive Visualization Workshop
# 
# Copyright (c) 2013-2025 Inviwo Foundation
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

# Add all external projects specified in cmake string IVW_EXTERNAL_PROJECTS
function(ivw_add_external_projects)
    foreach(project_root_path ${IVW_EXTERNAL_PROJECTS})
        string(STRIP ${project_root_path} project_root_path)
        get_filename_component(FOLDER_NAME ${project_root_path} NAME)
        add_subdirectory(${project_root_path} ${CMAKE_CURRENT_BINARY_DIR}/ext_${FOLDER_NAME})
    endforeach()
endfunction()
