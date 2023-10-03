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

 # Adds special qt dependency and includes package variables to the project
macro(ivw_qt_add_to_install ivw_comp)
    message(DEPRECATION "Use ivw_default_install_targets with target as argument instead")
    list(APPEND ivw_install_list
        "${CMAKE_CURRENT_BINARY_DIR};${CMAKE_PROJECT_NAME};Application;/"
    )
endmacro()

function(ivw_configure_application_module_dependencies target)
    message(FATAL_ERROR "removed infavor of linking to inviwo::module-system")
endfunction()


# A helper funtion to install targets.  deprecated
function(ivw_default_install_comp_targets comp)
    message(DEPRECATION "Use ivw_default_install_targets")
    ivw_default_install_targets(${ARGN})
endfunction()

macro(ivw_project project_name)
    message(DEPRECATION "Use project")
    project(${project_name} ${ARGN})
endmacro()

macro(ivw_set_cpack_name cpack_name)
    message(DEPRECATION "not used anymore")
endmacro()

macro(add_dependency_libs_to_module)
    message(FATAL_ERROR "use target_link_libraries")
endmacro()

macro(ivw_add_dependency_directories)
    message(FATAL_ERROR "Avoid using link directories.")
endmacro()

macro(ivw_include_directories)
    message(DEPRECATION "Use target_include_directories")
    include_directories(${ARGN})
endmacro()

macro(ivw_link_directories)
    message(DEPRECATION "Avoid link directories")
    link_directories("${ARGN}")
endmacro()

macro(ivw_add_link_flags)
    message(FATAL_ERROR "use target properties")
endmacro()

macro(ivw_add_definition def)
    message(FATAL_ERROR "use target_compile_definitions")
endmacro()
macro(ivw_add_definition_to_list def)
    message(FATAL_ERROR "use target_compile_definitions with INTERFACE")
endmacro()
macro(ivw_add_dependencies_on_target target)
    message(FATAL_ERROR "use target_link_libraries")
endmacro()
macro(ivw_define_qt_definitions)
    message(FATAL_ERROR "Not needed when using target_link_libraries")
endmacro()

# Wrap Qt CPP to create MOC files
macro(ivw_qt_wrap_cpp retval)
    message(DEPRECATION "Use qt5_wrap_cpp")
    qt5_wrap_cpp(the_list ${ARGN})
    set(${retval} ${the_list})
endmacro()

function(ivw_message)
    message(DEPRECATION "ivw_message is deprecated, just use message")
    message(${ARGV})
endfunction()

function(ivw_add_module_dependencies target)
    message(DEPRECATION "ivw_add_module_dependencies is deprecated,"
        " just use ivw_mod_name_to_alias and target_link_libraries")
    ivw_mod_name_to_alias(ivw_dep_targets ${ARGN})
    target_link_libraries(${target} PUBLIC ${ivw_dep_targets})
endfunction()

function(ivw_register_use_of_modules target)
    message(FATAL_ERROR "Replaced with: ivw_configure_application_module_dependencies")
endfunction()