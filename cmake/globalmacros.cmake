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

# Creates a inviwo module
macro(ivw_module project_name)
    project(${project_name} ${ARGN})
endmacro()

# Retrieve all enabled modules as a list
function(ivw_retrieve_all_modules module_list)
    set(${module_list} ${ivw_all_registered_modules} PARENT_SCOPE)
endfunction()

# Determine application dependencies. 
# Creates a list of enabled modules in executable directory if runtime
# module loading is enabled. Otherwise sets the registration macros and
# adds module dependencies
# Example: ivw_configure_application_module_dependencies(inviwo ${list_of_modules})
# The list of modules is usually fetched from ivw_retrieve_all_modules
function(ivw_configure_application_module_dependencies target)
    if(IVW_CFG_RUNTIME_MODULE_LOADING)
        # Specify which modules to load at runtime (all will be loaded if the file does not exist)
        ivw_create_enabled_modules_file(${target} ${ARGN})
        target_compile_definitions(${target} PUBLIC IVW_RUNTIME_MODULE_LOADING)
        # Dependencies to build before this project when they are changed.
        # Needed if modules are loaded at runtime since they should be built
        # when this project is set as startup project
        ivw_mod_name_to_alias(dep_targets ${ARGN})
        add_dependencies(${target} ${dep_targets})
    else()
        ivw_mod_name_to_reg(reg_targets ${ARGN})
        target_compile_definitions(${target} PUBLIC ${reg_targets})
        ivw_mod_name_to_alias(dep_targets ${ARGN})
        target_link_libraries(${target} PUBLIC ${dep_targets})
    endif()
endfunction()

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

function(ivw_private_setup_module_data)
    set(options CORE)
    set(oneValueArgs DIR BASE NAME GROUP VERSION)
    set(multiValueArgs "")
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    foreach(item IN LISTS oneValueArgs)
        if(NOT ARG_${item})
            message(FATAL_ERROR "ivw_private_setup_module_data: ${item} not specified")
        endif()
    endforeach()
    if(ARG_KEYWORDS_MISSING_VALUES) 
        message(FATAL_ERROR "ivw_private_setup_module_data: Missing values for keywords ${ARG_KEYWORDS_MISSING_VALUES}")
    endif()
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "ivw_private_setup_module_data: Unparsed arguments ${ARG_UNPARSED_ARGUMENTS}")
    endif()

    set(dir ${ARG_DIR})
    set(name ${ARG_NAME})
    set(module_path ${ARG_BASE})
    ivw_dir_to_mod_dep(mod ${dir})
    ivw_dir_to_mod_prefix(opt ${dir})           # OpenGL -> IVW_MODULE_OPENGL
    ivw_dir_to_module_taget_name(target ${dir}) # OpenGL -> inviwo-module-opengl

    ivw_private_get_ivw_module_include_path(${module_path}/${dir} includePrefix includePath orgName)
    # includePrefix -> inviwo/opengl
    # includePath -> ... inviwo/modules/opengl/include/inviwo/opengl
    # orgName -> inviwo

    # Get the classname with the right casing
    if(${ARG_CORE})
        set(class "InviwoCore")
        set(alias "inviwo::core")
        set(target "inviwo-core")
        set(header "inviwo/core/common/inviwocore.h")
        set(api "IVW_CORE_API")
        set(includePrefix "inviwo/core")
        set(includePath "${module_path}/${dir}/inviwo/core")
        set(orgName "inviwo")
        set(apiDefineInc "inviwo/core/common/inviwocoredefine.h")
        set(sharedLibInc "inviwo/core/common/coremodulesharedlibrary.h")
        set(sharedLibCpp ${CMAKE_BINARY_DIR}/modules/${dir}/src/common/${dir}modulesharedlibrary.cpp)
    else()
        set(class "${name}Module")
        set(alias "inviwo::module::${dir}")
        set(header "${includePrefix}/${dir}module.h")
        string(TOUPPER "${name}" u_module)
        set(api "IVW_MODULE_${u_module}_API")
        set(apiDefineInc ${includePrefix}/${dir}moduledefine.h)
        set(sharedLibInc ${includePrefix}/${dir}modulesharedlibrary.h)
        set(sharedLibCpp ${CMAKE_BINARY_DIR}/modules/${dir}/src/${dir}modulesharedlibrary.cpp)
    endif()
    set(sharedLibHpp ${CMAKE_BINARY_DIR}/modules/${dir}/include/${sharedLibInc})

    # Get module version
    set("${mod}_name"         "${name}"               CACHE INTERNAL "Module name")
    set("${mod}_dir"          "${dir}"                CACHE INTERNAL "Module dir")
    set("${mod}_base"         "${module_path}"        CACHE INTERNAL "Module base")
    set("${mod}_path"         "${module_path}/${dir}" CACHE INTERNAL "Module path")
    set("${mod}_group"        "${ARG_GROUP}"          CACHE INTERNAL "Module group")
    set("${mod}_opt"          "${opt}"                CACHE INTERNAL "Module cmake option")
    set("${mod}_target"       "${target}"             CACHE INTERNAL "Module target")
    set("${mod}_alias"        "${alias}"              CACHE INTERNAL "Module alias")
    set("${mod}_class"        "${class}"              CACHE INTERNAL "Module class")
    set("${mod}_modName"      "Inviwo${name}Module"   CACHE INTERNAL "Module mod name")
    set("${mod}_version"      "${ARG_VERSION}"        CACHE INTERNAL "Module version")
    set("${mod}_header"       "${header}"             CACHE INTERNAL "Module header")
    set("${mod}_incPrefix"    "${includePrefix}"      CACHE INTERNAL "Module include Prefix")
    set("${mod}_incPath"      "${includePath}"        CACHE INTERNAL "Module include Path")
    set("${mod}_orgName"      "${orgName}"            CACHE INTERNAL "Module Org Name")
    set("${mod}_api"          "${api}"                CACHE INTERNAL "API Macro")
    set("${mod}_apiDefineInc" "${apiDefineInc}"       CACHE INTERNAL "API header include")
    set("${mod}_sharedLibInc" "${sharedLibInc}"       CACHE INTERNAL "Shared lib include")
    set("${mod}_sharedLibCpp" "${sharedLibCpp}"       CACHE INTERNAL "Shared lib Source")
    set("${mod}_sharedLibHpp" "${sharedLibHpp}"       CACHE INTERNAL "Shared lib Header")

    # Check of there is a depends.cmake
    # Optionally defines: dependencies, aliases, protected, EnableByDefault
    # Save dependencies to INVIWO<NAME>MODULE_dependencies
    # Save aliases to INVIWO<NAME>MODULE_aliases
    # Save protected to INVIWO<NAME>MODULE_protected
    # Save EnableByDefault to INVIWO<NAME>MODULE_EnableByDefault
    set(dependencies "")
    set(aliases "")
    set(protected OFF)
    set(EnableByDefault OFF)
    set(Disabled OFF)
    set(DisabledReason "")
    if(EXISTS "${${mod}_path}/depends.cmake")
        include(${${mod}_path}/depends.cmake)
    endif()

    # set by ivw_enable_modules_if to enable non modules to force modules to build
    if(DEFINED ${mod}_enableExternal)
        set(EnableByDefault ${${mod}_enableExternal})
    endif()

    if(${ARG_CORE})
        set("${mod}_dependencies"    ${dependencies} CACHE INTERNAL "Module dependencies")
        set("${mod}_protected"       ON              CACHE INTERNAL "Protected Module")
        set("${mod}_enableByDefault" ON              CACHE INTERNAL "Enable module by default")
        set("${mod}_disabled"        OFF             CACHE INTERNAL "Disabled module")
        set("${mod}_disabledReason"  ""              CACHE INTERNAL "Reason for the module begin disabled")
    else()
        set("${mod}_dependencies"   "InviwoCoreModule;${dependencies}" CACHE INTERNAL "Module dependencies")
        set("${mod}_protected"       ${protected}                      CACHE INTERNAL "Protected Module")
        set("${mod}_enableByDefault" ${EnableByDefault}                CACHE INTERNAL "Enable module by default")
        set("${mod}_disabled"        ${Disabled}                       CACHE INTERNAL "Disabled module")
        set("${mod}_disabledReason"  ${DisabledReason}                 CACHE INTERNAL "Reason for the module begin disabled")
    endif()
    set("${mod}_aliases" ${aliases} CACHE INTERNAL "Module aliases")
    unset(dependencies)
    unset(aliases)
    unset(protected)

    # Check if there is a readme.md of the module. 
    # In that case set to INVIWO<NAME>MODULE_description
    if(EXISTS "${${mod}_path}/readme.md")
        file(READ "${${mod}_path}/readme.md" description)
        # truncate description since some readme files are quite substantial
        string(LENGTH "${description}" desc_len)
        if(desc_len GREATER 250)
            string(SUBSTRING "${description}" 0 250 description)
            string(JOIN "" description "${description}" "...")
        endif()
        # encode linebreaks, i.e. '\n', and semicolon in description for
        # proper handling in CMAKE
        encodeLineBreaks(cdescription ${description})
        set("${mod}_description" ${cdescription} CACHE INTERNAL "Module description")
    endif()
endfunction()

# Register modules
# Generate module options (which was not specified before) and,
# Sort directories based on dependencies inside directories
# defines:  (example project_name = OpenGL)
# INVIWOOPENGLMODULE_description  -> </readme.md>
# INVIWOOPENGLMODULE_dependencies -> </depends.cmake::dependencies>
# 
function(ivw_register_modules retval)
    # Collect all modules and information
    set(modules "")

    ivw_dir_to_mod_dep(mod core)
    list(APPEND modules ${mod})
    ivw_private_setup_module_data(CORE 
        NAME "Core" 
        VERSION "1.0.0" 
        GROUP "core" 
        DIR "core" 
        BASE ${IVW_SOURCE_DIR}
    )
    ivw_add_module_option_to_cache(${mod} ON)

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
        foreach(dir ${dirs})
            ivw_dir_to_mod_dep(mod ${dir})
            list(FIND modules ${mod} found)
            if(NOT ${found} EQUAL -1)
                message(WARNING "Module with name ${dir} already added at ${${mod}_path}")
                continue()
            endif()
            ivw_private_is_valid_module_dir(${module_path} ${dir} valid)
            if(${valid})
                list(APPEND modules ${mod})
                ivw_private_get_ivw_module_name(${module_path}/${dir}/CMakeLists.txt name)
                ivw_private_get_ivw_module_version(${module_path}/${dir}/CMakeLists.txt version)
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
    foreach(mod ${modules})
        ivw_add_module_option_to_cache(${mod} ${${mod}_enableByDefault})
    endforeach()

    # Find aliases
    set(aliases "")
    foreach(mod ${modules})
        foreach(alias ${${mod}_aliases})
            list(APPEND aliases ${alias})
            if(DEFINED alias_${alias}_mods)
                list(APPEND alias_${alias}_mods ${mod})
            else()
                set(alias_${alias}_mods ${mod})
            endif()
        endforeach()
    endforeach()

    # Substitute aliases
    foreach(mod ${modules})
        set(new_dependencies "")
        foreach(dependency ${${mod}_dependencies})
            list(FIND aliases ${dependency} found)
            if(NOT ${found} EQUAL -1)
                if(DEFINED ${${mod}_opt}_${dependency})
                    list(APPEND new_dependencies ${${${mod}_opt}_${dependency}})
                else()
                    # Find the best substitute
                    list(GET ${alias_${dependency}_mods} 0 new_mod)
                    set(new_dep ${${new_mod}_modName})
                    foreach(alias_mod ${alias_${dependency}_mods})
                        set(new_dep ${${alias_mod}_modName})
                        if(${${alias_mod}_opt}}) # if substitution is enabled we stick with that one.
                            break()
                        endif()
                    endforeach()
                    list(APPEND new_dependencies ${new_dep})
                    set(${${mod}_opt}_${dependency} "${new_dep}" CACHE STRING "Dependency")
                endif()
                set(alias_names "")
                foreach(alias_mod ${alias_${dependency}_mods})
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
    foreach(mod ${modules})
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
            foreach(dep ${${mod}_udependencies})
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
        foreach(dep ${${mod}_udependencies})
            if(${${dep}_disabled})
                set(disabled ON)
                set(disabledDep ${dep})
                break()
            endif()
        endforeach()

        if(disabled AND NOT ${mod}_disabled)
            set("${mod}_disabled" ON CACHE INTERNAL "Disabled module" FORCE)
            set("${mod}_disabledReason"  "Disabled due to a dependency toward the disabled module ${${disabledDep}_name}"
                CACHE INTERNAL "Reason for the module begin disabled"
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

    # Generate module registration file
    ivw_private_generate_module_registration_files(enabled_sorted_modules)
    
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

    # Save list of modules
    set(ivw_all_registered_modules ${ivw_module_names} CACHE INTERNAL "All registered inviwo modules")

    set(${retval} ${sorted_modules} PARENT_SCOPE)
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

# Creates VS folder structure
function(ivw_folder target folder_name)
    set_target_properties(${target} PROPERTIES FOLDER ${folder_name})
endfunction()

# Creates project module from name
# This it called from the inviwo module CMakeLists.txt 
# that is included from ivw_register_modules. 
function(ivw_create_module)
    set(options "NO_PCH" "QT")
    set(oneValueArgs "VERSION" "GROUP")
    set(multiValueArgs "PACKAGES" "FILES")
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include)
        set(LEGACY false)
    else()
        set(LEGACY true)
    endif()

    if(ARG_QT)
        set(qt "QT")
    else()
        set(qt "")
    endif()

    if(ARG_FILES) 
        set(files ${ARG_FILES})
    else()
        set(files ${ARG_UNPARSED_ARGUMENTS})
    endif()

    string(TOLOWER ${PROJECT_NAME} l_project_name)
    ivw_dir_to_mod_dep(mod ${l_project_name})  # opengl -> INVIWOOPENGLMODULE

    set(cmake_files "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/depends.cmake")
        list(APPEND cmake_files "${CMAKE_CURRENT_SOURCE_DIR}/depends.cmake")
    endif()
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/readme.md")
        list(APPEND cmake_files "${CMAKE_CURRENT_SOURCE_DIR}/readme.md")
    endif()
    source_group("CMake Files" FILES ${cmake_files})

    # Add module class files
    set(mod_class_files 
        # Add module creation function for dll/so loading
        ${CMAKE_CURRENT_BINARY_DIR}/src/${l_project_name}modulesharedlibrary.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/include/${${mod}_incPrefix}/${l_project_name}modulesharedlibrary.h
        $<$<BOOL:${LEGACY}>:${CMAKE_CURRENT_SOURCE_DIR}/${l_project_name}module.h>
        $<$<BOOL:${LEGACY}>:${CMAKE_CURRENT_SOURCE_DIR}/${l_project_name}module.cpp>
        $<$<BOOL:${LEGACY}>:${CMAKE_CURRENT_SOURCE_DIR}/${l_project_name}moduledefine.h>
    )

    remove_duplicates(ivw_unique_mod_files ${files} ${mod_class_files} ${cmake_files})
    set(mod_header_files ${ivw_unique_mod_files})
    list(FILTER mod_header_files INCLUDE REGEX ".*\\.h")

    set(mod_nonheader_files ${ivw_unique_mod_files})
    list(FILTER mod_nonheader_files EXCLUDE REGEX ".*\\.h")
    # Add module source files

    # Create library
    add_library(${${mod}_target})
    add_library(${${mod}_alias} ALIAS ${${mod}_target})

    target_sources(${${mod}_target} 
        PUBLIC 
        FILE_SET HEADERS
        TYPE HEADERS
        BASE_DIRS 
            ${CMAKE_CURRENT_BINARY_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        FILES 
            ${mod_header_files}
        PRIVATE
            ${mod_nonheader_files}
    )

    get_filename_component(base_parent ${${mod}_base} PATH)
    target_include_directories(${${mod}_target} PUBLIC 
        $<$<BOOL:${LEGACY}>:${${mod}_base}>
        $<$<BOOL:${LEGACY}>:${base_parent}>
    )

    # Define standard properties
    ivw_define_standard_definitions(${${mod}_opt} ${${mod}_target})
    ivw_define_standard_properties(${${mod}_target} ${qt})

    # Add dependencies from depends.cmake and InviwoCore
    ivw_mod_name_to_alias(ivw_dep_targets ${${mod}_dependencies})
    target_link_libraries(${${mod}_target} PUBLIC ${ivw_dep_targets})

    # Optimize compilation with pre-compiled headers
    if(NOT ARG_NO_PCH)
        ivw_compile_optimize_on_target(${${mod}_target})
    endif()

    # Add stuff to the installer
    ivw_install_module(MOD ${mod} PACKAGES ${ARG_PACKAGES})
    ivw_private_install_module_dirs()

    ivw_make_unittest_target("${${mod}_dir}" "${${mod}_target}")

    if(ARG_GROUP)
        ivw_folder(${${mod}_target} "${ARG_GROUP}")
    else()
        ivw_folder(${${mod}_target} "${${mod}_group}")
    endif()

    set_target_properties(${${mod}_target} PROPERTIES VERSION ${${mod}_version})
endfunction()

# Add all external projects specified in cmake string IVW_EXTERNAL_PROJECTS
function(ivw_add_external_projects)
    foreach(project_root_path ${IVW_EXTERNAL_PROJECTS})
        string(STRIP ${project_root_path} project_root_path)
        get_filename_component(FOLDER_NAME ${project_root_path} NAME)
        add_subdirectory(${project_root_path} ${CMAKE_CURRENT_BINARY_DIR}/ext_${FOLDER_NAME})
    endforeach()
endfunction()