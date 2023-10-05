#################################################################################
# 
# Inviwo - Interactive Visualization Workshop
# 
# Copyright (c) 2023 Inviwo Foundation
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

# Register modules
# Generate module options (which was not specified before) and,
# Sort directories based on dependencies inside directories
# defines:  (example project_name = OpenGL)
# INVIWOOPENGLMODULE_description  -> </readme.md>
# INVIWOOPENGLMODULE_dependencies -> </depends.cmake::dependencies>
# 
function(ivw_register_modules)
    set(options )
    set(oneValueArgs ALL_MODULES_OUT ENABLED_MODULES_OUT ENABLED_MODULE_TARGETS_OUT MODULE_REGISTRATION_FILE)
    set(multiValueArgs )
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "ivw_register_modules: Unparsed arguments ${ARG_UNPARSED_ARGUMENTS}")
    endif()
    if(ARG_KEYWORDS_MISSING_VALUES) 
        message(FATAL_ERROR "ivw_register_modules: Missing values for keywords ${ARG_KEYWORDS_MISSING_VALUES}")
    endif()

    # Collect all modules and information
    set(modules "")

    ivw_dir_to_mod_dep(mod core)
    list(APPEND modules ${mod})
    ivw_private_setup_core_data()
    set(${${mod}_opt} ON CACHE INTERNAL "Core module opt")

    foreach(module_path ${IVW_MODULE_DIR} ${IVW_EXTERNAL_MODULES})
        get_filename_component(group_name ${module_path} NAME)
        set(group_name "modules-${group_name}")

        # Check of there is a meta.cmake
        # Optionally defines: group_name
        if(EXISTS "${module_path}/meta.cmake")
            include("${module_path}/meta.cmake")
        endif()

        string(STRIP ${module_path} module_path)
        if(NOT EXISTS ${module_path})
             message(WARNING "External module path does not exist: '${module_path}'")
             continue()
        endif()
        if(NOT IS_DIRECTORY ${module_path})
             message(WARNING "External module path is not a directory: '${module_path}'")
             continue()
        endif()

        file(GLOB dirs RELATIVE ${module_path} ${module_path}/[^.]*)
        foreach(dir IN LISTS dirs)
            ivw_dir_to_mod_dep(mod ${dir})
            list(FIND modules ${mod} found)
            if(NOT ${found} EQUAL -1)
                message(WARNING "Module with name ${dir} already added at ${${mod}_path}")
                continue()
            endif()
            ivw_private_is_valid_module_dir(
                PATH ${module_path}
                DIR ${dir}
                VALID_VAR valid_module_dir
                NAME_VAR name
                VERSION_VAR version
            )
            if(valid_module_dir)
                list(APPEND modules ${mod})
                ivw_private_setup_module_data(
                    NAME ${name}
                    GROUP ${group_name}
                    VERSION ${version}
                    DIR ${dir}
                    BASE ${module_path}
                )
            endif()
        endforeach()
    endforeach()

    # Add modules to cmake cache
    foreach(mod IN LISTS modules)
        ivw_add_module_option_to_cache(${mod} ${${mod}_enableByDefault})
    endforeach()

    # Find aliases
    set(aliases "")
    foreach(mod IN LISTS modules)
        foreach(alias IN LISTS ${mod}_aliases)
            list(APPEND aliases ${alias})
            if(DEFINED alias_${alias}_mods)
                list(APPEND alias_${alias}_mods ${mod})
            else()
                set(alias_${alias}_mods ${mod})
            endif()
        endforeach()
    endforeach()

    # Substitute aliases
    foreach(mod IN LISTS modules)
        set(new_dependencies "")
        foreach(dependency IN LISTS ${mod}_dependencies)
            list(FIND aliases ${dependency} found)
            if(NOT ${found} EQUAL -1)
                if(DEFINED ${${mod}_opt}_${dependency})
                    list(APPEND new_dependencies ${${${mod}_opt}_${dependency}})
                else()
                    # Find the best substitute
                    list(GET ${alias_${dependency}_mods} 0 new_mod)
                    set(new_dep ${${new_mod}_modName})
                    foreach(alias_mod IN LISTS alias_${dependency}_mods)
                        set(new_dep ${${alias_mod}_modName})
                        if(${${alias_mod}_opt}}) # if substitution is enabled we stick with that one.
                            break()
                        endif()
                    endforeach()
                    list(APPEND new_dependencies ${new_dep})
                    set(${${mod}_opt}_${dependency} "${new_dep}" CACHE STRING "Dependency")
                endif()
                set(alias_names "")
                foreach(alias_mod IN LISTS alias_${dependency}_mods)
                    list(APPEND alias_names ${${alias_mod}_modName})
                endforeach()
                set_property(CACHE ${${mod}_opt}_${dependency} PROPERTY STRINGS ${alias_names})
            else()
                list(APPEND new_dependencies ${dependency})
            endif()
        endforeach()
        # Validate that there only are module dependencies
        ivw_private_filter_dependency_list(new_dependencies ${${mod}_name} ${new_dependencies})
        ivw_private_check_dependency_list(new_dependencies modules ${${mod}_name} ${new_dependencies})
        set("${mod}_dependencies" ${new_dependencies} CACHE INTERNAL "Module dependencies")
        ivw_mod_name_to_mod_dep(udependencies ${new_dependencies})
        set("${mod}_udependencies" ${udependencies} CACHE INTERNAL "Module uppercase dependencies")
    endforeach()
    
    # Add module versions dependencies
    foreach(mod IN LISTS modules)
        set(dependencies_version "")
        foreach(dependency IN LISTS ${mod}_dependencies)
            ivw_mod_name_to_mod_dep(dep ${dependency})
            list(FIND modules ${dep} found)
            if(NOT ${found} EQUAL -1)
                list(GET modules ${found} module)
                list(APPEND dependencies_version ${${module}_version})
            else()
                # Dependency was not found, not an inviwo module...
                # We do not take responsibility for external library versions.
                # Distribute the dependency along with the library!
                message(WARNING "${${mod}_name}: ${dependency} dependency not found")
            endif()
        endforeach()
        set("${mod}_dependenciesversion" ${dependencies_version} CACHE INTERNAL "Module dependency versions")
    endforeach()

    # Sort modules by dependencies
    ivw_topological_sort(modules _udependencies sorted_modules)

    # enable dependencies transitively if they are disabled
    ivw_reverse_list_copy(sorted_modules rev_sorted_modules)
    foreach(mod IN LISTS rev_sorted_modules)
        if(${${mod}_opt})
            foreach(dep IN LISTS ${mod}_udependencies)
                if(NOT ${${dep}_opt})
                    ivw_add_module_option_to_cache(${dep} ON FORCE)
                    message(STATUS "${${dep}_opt} was set to build, "
                        "due to dependency towards module ${${mod}_name}")
                endif()
            endforeach()
        endif()
    endforeach()

    # Transitively disable modules if they depend on a disabled module
    foreach(mod IN LISTS sorted_modules)
        set(disabled OFF)
        set(disabledDep "")
        foreach(dep IN LISTS ${mod}_udependencies)
            if(${${dep}_disabled})
                set(disabled ON)
                set(disabledDep ${dep})
                break()
            endif()
        endforeach()

        if(disabled AND NOT ${mod}_disabled)
            set("${mod}_disabled" ON)
            set("${mod}_disabledReason" 
                "Disabled due to a dependency toward the disabled module ${${disabledDep}_name}"
            )
        endif()
    endforeach()

    ivw_format(STATUS 
        FORMAT "\n{0:<30} {1:>3} {2:^8} {3:^8} {4:20} {5}"
        ARGUMENTS "Name" "On" "Disabled" "Default" "Group" "Dependencies"
    )
    ivw_format(STATUS FORMAT "{:=<90}" ARGUMENTS "=")
    foreach(mod IN LISTS sorted_modules)
        ivw_remove_from_list(deps ${mod}_dependencies InviwoCoreModule)
        ivw_mod_name_to_name(deps ${deps})
        if(${${mod}_disabled})
            ivw_join(";" ", " deps ${deps} "**${${mod}_disabledReason}**")
        else()
            ivw_join(";" ", " deps ${deps})
        endif()

        ivw_format(STATUS
            FORMAT "{0:<30} {1:>3} {2:^8} {3:^8} {4:20} {5}"
            ARGUMENTS
                ${${mod}_name}
                ${${${mod}_opt}}
                ${${mod}_disabled}
                ${${mod}_enableByDefault}
                "${${mod}_group} "
                "${deps} "
        )
    endforeach()
    ivw_format(STATUS FORMAT "{:=<90}" ARGUMENTS "=")

    ivw_copy_if(on_sorted_modules LIST sorted_modules EVAL PROJECTOR _opt)
    ivw_copy_if(enabled_sorted_modules LIST on_sorted_modules NOT PROJECTOR _disabled)
    
    # Add enabled modules in sorted order
    set(ivw_module_names "")
    foreach(mod IN LISTS enabled_sorted_modules)
        message(STATUS "create module: ${${mod}_name}")
        list(APPEND CMAKE_MESSAGE_INDENT "    ")

        add_subdirectory(${${mod}_path} ${IVW_BINARY_DIR}/modules/${${mod}_dir})
        list(APPEND ivw_module_names ${${mod}_modName})
        ivw_private_generate_module_registration_file(${mod})

        list(POP_BACK CMAKE_MESSAGE_INDENT)
    endforeach()


    if(ARG_MODULE_REGISTRATION_FILE)
        # Generate module registration file
        ivw_private_generate_module_registration_files(
            DESTINATION ${ARG_MODULE_REGISTRATION_FILE}
            MODULES ${enabled_sorted_modules}
        )
    endif()


    if(ARG_ALL_MODULES_OUT)
        set(${ARG_ALL_MODULES_OUT} ${sorted_modules} PARENT_SCOPE)
    endif()
    if(ARG_ENABLED_MODULES_OUT)
        set(${ARG_ENABLED_MODULES_OUT} ${enabled_sorted_modules} PARENT_SCOPE)
    endif()
    if(ARG_ENABLED_MODULE_TARGETS_OUT)
        ivw_mod_name_to_alias(enabled_sorted_module_targets ${enabled_sorted_modules})
        set(${ARG_ENABLED_MODULE_TARGETS_OUT} ${enabled_sorted_module_targets} PARENT_SCOPE)
    endif()

endfunction()