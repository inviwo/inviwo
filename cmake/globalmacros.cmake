#################################################################################
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2013-2017 Inviwo Foundation
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

#--------------------------------------------------------------------
# Creates a inviwo module
macro(ivw_module project_name)
    project(${project_name} ${ARGN})
endmacro()

#--------------------------------------------------------------------
# Retrieve all enabled modules as a list
function(ivw_retrieve_all_modules module_list)
    set(${module_list} ${ivw_all_registered_modules} PARENT_SCOPE)
endfunction()

#--------------------------------------------------------------------
# Determine application dependencies. 
# Creates a list of enabled modules in executable directory if runtime
# module loading is enabled. Otherwise sets the registration macros and
# adds module dependencies
# Example: ivw_configure_application_module_dependencies(inviwo ${list_of_modules})
# The list of modules is usually fetched from ivw_retrieve_all_modules
function(ivw_configure_application_module_dependencies target)
    if(IVW_RUNTIME_MODULE_LOADING)
        # Specify which modules to load at runtime (all will be loaded if the file does not exist)
        ivw_create_enabled_modules_file(${target} ${ARGN})
        target_compile_definitions(${target} PUBLIC IVW_RUNTIME_MODULE_LOADING)
        # Dependencies to build before this project when they are changed.
        # Needed if modules are loaded at runtime since they should be built
        # when this project is set as startup project
        ivw_mod_name_to_target_name(dep_targets ${ARGN})
        add_dependencies(${target} ${dep_targets})
    else()
        ivw_mod_name_to_reg(reg_targets ${ARGN})
        target_compile_definitions(${target} PUBLIC ${reg_targets})
        ivw_mod_name_to_alias(dep_targets ${ARGN})
        target_link_libraries(${target} PUBLIC ${dep_targets})
    endif()
endfunction()

#--------------------------------------------------------------------
# Create a file ("executable_name-enabled-modules.txt") in the binary output directory.
# The application can use the file to check the enabled modules at runtime.
# Usage: ivw_create_enabled_modules_file("application_name" ${enabled_modules})
# where enabled_modules is a list of module names (i.e. InviwoBaseModule)
function(ivw_create_enabled_modules_file executable_name)
    set(enabled_modules "")
    foreach(mod ${ARGN})  
        ivw_mod_name_to_dir(mod_name ${mod})
        set(enabled_modules "${enabled_modules}${mod_name}\n") 
    endforeach()
    if(MSVC OR XCODE_VERSION)
        # Multi-configuration generators (VS, Xcode) append a per-configuration 
        # subdirectory to the specified directory
        foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
            file(WRITE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${OUTPUTCONFIG}/${executable_name}-enabled-modules.txt" ${enabled_modules})
        endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
    else()
        file(WRITE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${executable_name}-enabled-modules.txt" ${enabled_modules})
    endif()
endfunction()

# Generate header for modules
function(ivw_generate_module_paths_header)
    set(dirs "")
    foreach(dir ${IVW_ROOT_DIR}/modules ${IVW_EXTERNAL_MODULES})
        if(IS_DIRECTORY ${dir})
            list(APPEND dirs ${dir})
        else()
            message("Path to external module is not a directory (${dir})")
        endif()
    endforeach()

    list_to_longstringvector(vec ${dirs})
    list(LENGTH IVW_EXTERNAL_MODULES count)
    math(EXPR count "${count}+1")
    set(paths "const std::array<const std::string, ${count}> inviwoModulePaths_ = {${vec}}")
    set(IVW_MODULES_PATHS_ARRAY ${paths})

    configure_file(${IVW_CMAKE_TEMPLATES}/inviwomodulespaths_template.h 
                   ${CMAKE_BINARY_DIR}/modules/_generated/inviwomodulespaths.h @ONLY)
endfunction()

#--------------------------------------------------------------------
# Generate a module registration header file (with configure file etc)
function(ivw_private_generate_module_registration_file modules_var)
    # For runtime loading export a module factory function for all modules.
    # Function will be requested by the application after loading the library (dll/so)
    # Does not require modules to be linked to the application
    # For static loading generate function for creating modules in a single function
    # Requires all modules to be linked to the application

    set(static_headers "")
    set(static_functions "")

    foreach(mod ${${modules_var}})
        if(NOT ${${mod}_opt})
            continue()
        endif()

        ivw_mod_name_to_dir(module_dependencies ${${mod}_dependencies})
        list_to_stringvector(module_depends_vector ${module_dependencies})
        list_to_stringvector(module_depends_version_vector ${${mod}_dependenciesversion})
        list_to_stringvector(module_alias_vector ${${mod}_aliases})
        string(TOUPPER "${${mod}_class}" u_module)
        if(${${mod}_protected})
            set(module_protected "ProtectedModule::on")
        else()
            set(module_protected "ProtectedModule::off")
        endif()

        get_filename_component(path ${${mod}_header} DIRECTORY)

        list(APPEND static_headers
            "#ifdef REG_${mod}\n"
            "#include <${path}/${${mod}_dir}modulesharedlibrary.h>\n"
            "#endif\n"
        )
        list(APPEND static_functions
            "    #ifdef REG_${mod}\n" 
            "    modules.emplace_back(create${${mod}_class}Module())__SEMICOLON__\n"
            "    #endif\n"
        )

        set(fuction_args
            "        \"${${mod}_class}\", // Module name \n"
            "        \"${${mod}_version}\", // Module version\n"
            "        \"${${mod}_description}\", // Description\n" 
            "        \"${IVW_VERSION}\", // Inviwo core version when built \n" 
            "        ${module_depends_vector}, // Dependencies\n" 
            "        ${module_depends_version_vector}, // Version number of dependencies\n"
            "        ${module_alias_vector}, // List of aliases\n"
            "        ${module_protected} // protected"
        )

        join(";" "" fuction_args ${fuction_args})
        string(REPLACE "__LINEBREAK__" "\\n\"\n        \"" fuction_args "${fuction_args}")
        string(REPLACE "__SEMICOLON__" ";" fuction_args "${fuction_args}")

        set(MODULE ${${mod}_class})
        if(${${mod}_class} STREQUAL "Core")
            set(MODULE_NAME "InviwoCore")
            set(API_HEADER inviwo/core/common/inviwocoredefine.h)
            set(API_DEFINE IVW_CORE_API)
        else()
            set(MODULE_NAME "${${mod}_class}Module")
            set(API_HEADER modules/${${mod}_dir}/${${mod}_dir}moduledefine.h)
            set(API_DEFINE IVW_MODULE_${u_module}_API)
        endif()

        set(U_MODULE ${u_module})
        set(MODULE_HEADER ${${mod}_header})
        set(MODULE_ARGS ${fuction_args})
        set(LIBRARY_HEADER ${path}/${${mod}_dir}modulesharedlibrary.h)

        configure_file(
            ${IVW_CMAKE_TEMPLATES}/mod_shared_library_template.cpp 
            ${CMAKE_BINARY_DIR}/modules/_generated/${path}/${${mod}_dir}modulesharedlibrary.cpp 
            @ONLY
        )
        configure_file(
            ${IVW_CMAKE_TEMPLATES}/mod_shared_library_template.h
            ${CMAKE_BINARY_DIR}/modules/_generated/${path}/${${mod}_dir}modulesharedlibrary.h 
            @ONLY
        )
    endforeach()

    join(";" "" static_headers ${static_headers})
    join(";" "" static_functions ${static_functions})
    string(REPLACE "__LINEBREAK__" "\\n\"\n        \"" static_functions "${static_functions}")
    string(REPLACE "__SEMICOLON__" ";" static_functions "${static_functions}")

    set(MODULE_HEADERS "${static_headers}")
    set(MODULE_CLASS_FUNCTIONS "${static_functions}")
    configure_file(
        ${IVW_CMAKE_TEMPLATES}/mod_registration_template.h 
        ${CMAKE_BINARY_DIR}/modules/_generated/moduleregistration.h 
        @ONLY
    )
endfunction()

function(ivw_private_create_pyconfig modulepaths activemodules)
    # template vars:
    set(MODULEPATHS ${modulepaths})
    set(ACTIVEMODULES ${activemodules})

    find_package(Git QUIET)
    if(GIT_FOUND)
        ivw_debug_message(STATUS "git found: ${GIT_EXECUTABLE}")
    else()
        set(GIT_EXECUTABLE "")
    endif()

    configure_file(${IVW_CMAKE_TEMPLATES}/pyconfig_template.ini 
                   ${CMAKE_BINARY_DIR}/pyconfig.ini @ONLY)
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
    set(oneValueArgs DIR BASE)
    set(multiValueArgs "")
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(dir ${ARG_DIR})
    set(module_path ${ARG_BASE})
    ivw_dir_to_mod_dep(mod ${dir})
    ivw_dir_to_mod_prefix(opt ${dir})           # OpenGL -> IVW_MODULE_OPENGL
    ivw_dir_to_module_taget_name(target ${dir}) # OpenGL -> inviwo-module-opengl
    # Get the classname with the right casing
    if(${ARG_CORE})
        set(name "Core")
        set(alias "inviwo::core")
        set(header "inviwo/core/common/inviwocore.h")
    else()
        ivw_private_get_ivw_module_name(${module_path}/${dir}/CMakeLists.txt name)
        set(alias "inviwo::module::${dir}")
        set(header "modules/${dir}/${dir}module.h")
    endif()
    # Get module version
    ivw_private_get_ivw_module_version(${module_path}/${dir}/CMakeLists.txt version)
    set("${mod}_dir"    "${dir}"                CACHE INTERNAL "Module dir")
    set("${mod}_base"   "${module_path}"        CACHE INTERNAL "Module base")
    set("${mod}_path"   "${module_path}/${dir}" CACHE INTERNAL "Module path")
    set("${mod}_opt"    "${opt}"                CACHE INTERNAL "Module cmake option")
    set("${mod}_target" "${target}"             CACHE INTERNAL "Module target")
    set("${mod}_alias"  "${alias}"              CACHE INTERNAL "Module alias")
    set("${mod}_class"  "${name}"               CACHE INTERNAL "Module class")
    set("${mod}_name"   "Inviwo${name}Module"   CACHE INTERNAL "Module name")
    set("${mod}_version" "${version}"           CACHE INTERNAL "Module version")
    set("${mod}_header"  "${header}"            CACHE INTERNAL "Module header")

    # Check of there is a depends.cmake
    # Optionally defines: dependencies, aliases, protected
    # Save dependencies to INVIWO<NAME>MODULE_dependencies
    # Save aliases to INVIWO<NAME>MODULE_aliases
    # Save protected to INVIWO<NAME>MODULE_protected
    set(dependencies "")
    set(aliases "")
    set(protected OFF)
    if(EXISTS "${${mod}_path}/depends.cmake")
        include(${${mod}_path}/depends.cmake)
    endif()
    if(${ARG_CORE})
        set("${mod}_dependencies" ${dependencies} CACHE INTERNAL "Module dependencies")
        set("${mod}_protected"    ON              CACHE INTERNAL "Protected Module")
    else()
        set("${mod}_dependencies" "InviwoCoreModule;${dependencies}" CACHE INTERNAL "Module dependencies")
        set("${mod}_protected"     ${protected}                      CACHE INTERNAL "Protected Module")
    endif()
    set("${mod}_aliases" ${aliases} CACHE INTERNAL "Module aliases")
    unset(dependencies)
    unset(aliases)
    unset(protected)

    # Check if there is a readme.md of the module. 
    # In that case set to INVIWO<NAME>MODULE_description
    if(EXISTS "${${mod}_path}/readme.md")
        file(READ "${${mod}_path}/readme.md" description)
        # encode linebreaks, i.e. '\n', and semicolon in description for
        # proper handling in CMAKE
        encodeLineBreaks(cdescription ${description})
        set("${mod}_description" ${cdescription} CACHE INTERNAL "Module description")
    endif()
endfunction()

#--------------------------------------------------------------------
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
    ivw_private_setup_module_data(CORE DIR "core" BASE ${IVW_SOURCE_DIR})
    ivw_add_module_option_to_cache(${${mod}_dir} ON FALSE)

    foreach(module_path ${IVW_MODULE_DIR} ${IVW_EXTERNAL_MODULES})
        string(STRIP ${module_path} module_path)
        file(GLOB dirs RELATIVE ${module_path} ${module_path}/[^.]*)
        foreach(dir ${dirs})
            ivw_dir_to_mod_dep(mod ${dir})
            list(FIND modules ${mod} found)
            if(NOT ${found} EQUAL -1)
                message("Module with name ${dir} already added at ${${mod}_path}")
                continue()
            endif()
            ivw_private_is_valid_module_dir(${module_path} ${dir} valid)
            if(${valid})
                ivw_debug_message(STATUS "register module: ${dir}")
                list(APPEND modules ${mod})
                ivw_private_setup_module_data(DIR ${dir} BASE ${module_path})
            endif()
        endforeach()
    endforeach()

    # Add modules to cmake cache
    foreach(mod ${modules})
        lowercase(default_dirs ${ivw_default_modules})
        list(FIND default_dirs ${${mod}_dir} index)
        if(NOT index EQUAL -1)
            ivw_add_module_option_to_cache(${${mod}_dir} ON FALSE)
        else()
            ivw_add_module_option_to_cache(${${mod}_dir} OFF FALSE)
        endif()
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
                    set(new_dep ${${new_mod}_name})
                    foreach(alias_mod ${alias_${dependency}_mods})
                        set(new_dep ${${alias_mod}_name})
                        if(${${alias_mod}_opt}}) # if substitution is enabled we stick with that one.
                            break()
                        endif()
                    endforeach()
                    list(APPEND new_dependencies ${new_dep})
                    set(${${mod}_opt}_${dependency} "${new_dep}" CACHE STRING "Dependency")
                endif()
                set(alias_names "")
                foreach(alias_mod ${alias_${dependency}_mods})
                    list(APPEND alias_names ${${alias_mod}_name})
                endforeach()
                set_property(CACHE ${${mod}_opt}_${dependency} PROPERTY STRINGS ${alias_names})
            else()
                list(APPEND new_dependencies ${dependency})
            endif()
        endforeach()
        # Validate that there only are module dependencies
        ivw_private_filter_dependency_list(new_dependencies ${${mod}_class} ${new_dependencies})
        ivw_private_check_dependency_list(new_dependencies modules ${${mod}_class} ${new_dependencies})
        set("${mod}_dependencies" ${new_dependencies} CACHE INTERNAL "Module dependencies")
        ivw_mod_name_to_mod_dep(udependencies ${new_dependencies})
        set("${mod}_udependencies" ${udependencies} CACHE INTERNAL "Module uppercase dependencies")
    endforeach()
    
    # Add module versions dependencies
    foreach(mod ${modules})
        set(dependencies_version "")
        foreach(dependency ${${mod}_dependencies})
            ivw_mod_name_to_mod_dep(dep ${dependency})
            list(FIND modules ${dep} found)
            if(NOT ${found} EQUAL -1)
                list(GET modules ${found} module)
                list(APPEND dependencies_version ${${module}_version})
            else()
                # Dependency was not found, not an inviwo module...
                # We do not take responsibility for external library versions.
                # Distribute the dependency along with the library!
                # ivw_message("${${mod}_class}: ${dependency} dependency not found")
            endif()
        endforeach()
        set("${mod}_dependenciesversion" ${dependencies_version} CACHE INTERNAL "Module dependency versions")
    endforeach()

    # Sort modules by dependencies
    ivw_topological_sort(modules _udependencies sorted_modules)

    # enable dependencies
    ivw_reverse_list_copy(sorted_modules rev_sorted_modules)
    foreach(mod ${rev_sorted_modules})
        if(${${mod}_opt})
            foreach(dep ${${mod}_udependencies})
                if(NOT ${${dep}_opt})
                    ivw_add_module_option_to_cache(${${dep}_dir} ON TRUE)
                    message(STATUS "${${dep}_opt} was set to build, "
                        "due to dependency towards ${${mod}_opt}")
                endif()
            endforeach()
        endif()
    endforeach()
    
    # Generate module registration file
    ivw_private_generate_module_registration_file(sorted_modules)
    
    # Add enabled modules in sorted order
    set(ivw_module_names "")
    foreach(mod ${sorted_modules})
        if(${${mod}_opt})
            add_subdirectory(${${mod}_path} ${IVW_BINARY_DIR}/modules/${${mod}_dir})
            list(APPEND ivw_module_names ${${mod}_name})
        endif()
    endforeach()

    # Save list of modules
    set(ivw_all_registered_modules ${ivw_module_names} CACHE INTERNAL "All registered inviwo modules")

    # Save information for python tools.
    ivw_mod_name_to_class(ivw_module_classes ${ivw_module_names})
    ivw_private_create_pyconfig("${IVW_MODULE_DIR};${IVW_EXTERNAL_MODULES}" "${ivw_module_classes}")

    set(${retval} ${sorted_modules} PARENT_SCOPE)
endfunction()

#--------------------------------------------------------------------
# Set module build option to true if the owner is built
function(ivw_add_build_module_dependency the_module the_owner)
    ivw_dir_to_mod_prefix(mod_name ${the_module})
    first_case_upper(dir_name_cap ${the_module})
    if(${the_owner} AND NOT ${mod_name})
        ivw_add_module_option_to_cache(${the_module} ON TRUE)
        message(STATUS "${mod_name} was set to build, due to dependency towards ${the_owner}")
    endif()
endfunction()

#--------------------------------------------------------------------
# Set module build option to true
function(ivw_enable_module the_module)
    ivw_add_module_option_to_cache(${the_module} ON FALSE)
endfunction()

#--------------------------------------------------------------------
# Creates source group structure recursively
function(ivw_group group_name)
    foreach(currentSourceFile ${ARGN})
        if(NOT IS_ABSOLUTE ${currentSourceFile})
            set(currentSourceFile ${CMAKE_CURRENT_SOURCE_DIR}/${currentSourceFile})
        endif()
        string(REPLACE "include/inviwo/" "src/" currentSourceFileModified ${currentSourceFile})
        file(RELATIVE_PATH folder ${CMAKE_CURRENT_SOURCE_DIR} ${currentSourceFileModified})
        get_filename_component(folder ${folder} PATH)

        if(group_name STREQUAL "Test Files")
            string(REPLACE "tests/unittests" "" folder ${folder})
        endif()

        if(NOT folder STREQUAL "")
            string(REGEX REPLACE "/+$" "" folderlast ${folder})
            string(REPLACE "/" "\\" folderlast ${folderlast})
            source_group("${group_name}\\${folderlast}" FILES ${currentSourceFile})
        else()
            source_group("${group_name}" FILES ${currentSourceFile})
        endif(NOT folder STREQUAL "")
    endforeach(currentSourceFile ${ARGN})
endfunction()

#--------------------------------------------------------------------
# Creates VS folder structure
function(ivw_folder target folder_name)
    set_target_properties(${target} PROPERTIES FOLDER ${folder_name})
endfunction()

#--------------------------------------------------------------------
# Creates project module from name
# This it called from the inviwo module CMakeLists.txt 
# that is included from ivw_register_modules. 
function(ivw_create_module)
    string(TOLOWER ${PROJECT_NAME} l_project_name)
    ivw_debug_message(STATUS "create module: ${PROJECT_NAME}")
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
        ${CMAKE_CURRENT_SOURCE_DIR}/${l_project_name}module.h
        ${CMAKE_CURRENT_SOURCE_DIR}/${l_project_name}module.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/${l_project_name}moduledefine.h
        # Add module creation function for dll/so loading
        ${CMAKE_BINARY_DIR}/modules/_generated/modules/${l_project_name}/${l_project_name}modulesharedlibrary.cpp
        ${CMAKE_BINARY_DIR}/modules/_generated/modules/${l_project_name}/${l_project_name}modulesharedlibrary.h
    )
    remove_duplicates(ivw_unique_mod_files ${ARGN} ${mod_class_files} ${cmake_files})

    # Create library
    add_library(${${mod}_target} ${ivw_unique_mod_files})
    add_library(${${mod}_alias} ALIAS ${${mod}_target})
    
    # Define standard properties
    ivw_define_standard_definitions(${${mod}_opt} ${${mod}_target})
    ivw_define_standard_properties(${${mod}_target})

    # Add dependencies from depends.cmake and InviwoCore
    ivw_mod_name_to_alias(ivw_dep_targets ${${mod}_dependencies})
    target_link_libraries(${${mod}_target} PUBLIC ${ivw_dep_targets})

    # Optimize compilation with pre-compilied headers based on inviwo-core
    ivw_compile_optimize_inviwo_core_on_target(${${mod}_target})

    # Add stuff to the installer
    ivw_default_install_targets(${${mod}_target})
    ivw_private_install_module_dirs()

    # Make package (for other modules to find)
    ivw_make_package($Inviwo${PROJECT_NAME}Module ${${mod}_target})
    ivw_make_unittest_target("${${mod}_dir}" "${${mod}_target}")
endfunction()

#--------------------------------------------------------------------
# Add all external projects specified in cmake string IVW_EXTERNAL_PROJECTS
function(ivw_add_external_projects)
    foreach(project_root_path ${IVW_EXTERNAL_PROJECTS})
        string(STRIP ${project_root_path} project_root_path)
        get_filename_component(FOLDER_NAME ${project_root_path} NAME)
        add_subdirectory(${project_root_path} ${CMAKE_CURRENT_BINARY_DIR}/ext_${FOLDER_NAME})
    endforeach()
endfunction()

#-------------------------------------------------------------------#
#                        Precompile headers                         #
#-------------------------------------------------------------------#
# Set header ignore paths for cotire
function(ivw_cotire_ignore_on_target target)
    get_target_property(COTIRE_PREFIX_HEADER_IGNORE_PATH ${target} COTIRE_PREFIX_HEADER_IGNORE_PATH)
    if(NOT COTIRE_PREFIX_HEADER_IGNORE_PATH)
        set(COTIRE_PREFIX_HEADER_IGNORE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()

    list(APPEND COTIRE_PREFIX_HEADER_IGNORE_PATH "${IVW_EXTENSIONS_DIR}/warn")
    list(REMOVE_DUPLICATES COTIRE_PREFIX_HEADER_IGNORE_PATH)

    set_target_properties(${target} PROPERTIES COTIRE_PREFIX_HEADER_IGNORE_PATH "${COTIRE_PREFIX_HEADER_IGNORE_PATH}")  
endfunction()

# Optimize compilation with pre-compilied headers from inviwo core
function(ivw_compile_optimize_inviwo_core_on_target target)
    if(PRECOMPILED_HEADERS)
        ivw_cotire_ignore_on_target(${target})
        set_target_properties(${target} PROPERTIES COTIRE_ADD_UNITY_BUILD FALSE)
        get_target_property(_prefixHeader inviwo-core COTIRE_CXX_PREFIX_HEADER)
        set_target_properties(${target} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "${_prefixHeader}")
        cotire(${target})
    endif()
endfunction()

# Optimize compilation with pre-compilied headers
function(ivw_compile_optimize_on_target target)
    if(PRECOMPILED_HEADERS)
        ivw_cotire_ignore_on_target(${target})
        set_target_properties(${target} PROPERTIES COTIRE_ADD_UNITY_BUILD FALSE)
        set_target_properties(${target} PROPERTIES COTIRE_PREFIX_HEADER_INCLUDE_PATH "${IVW_EXTENSIONS_DIR}")
        cotire(${target})
    endif()
endfunction()