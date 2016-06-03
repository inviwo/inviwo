 #################################################################################
 #
 # Inviwo - Interactive Visualization Workshop
 #
 # Copyright (c) 2013-2015 Inviwo Foundation
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

#--------------------------------------------------------------------
# Creates project with initial variables
# Creates a CMake projects
# Defines:  (example project_name = OpenGL)
# _projectName -> OpenGL
# _allIncludes -> ""
# _allIncludeDirs -> ""
# _allLibsDir -> ""
# _allLibs -> ""
# _allDefinitions -> ""
# _allLinkFlags -> ""
# _allPchDirs -> ""
# _cpackName -> modules or qt_modules if QT
# _pchDisabledForThisModule -> FALSE 
macro(ivw_project project_name)
    project(${project_name})
    set(_projectName ${project_name})
    set(_allIncludes "")
    set(_allIncludeDirs "")
    set(_allLibsDir "")
    set(_allLibs "")
    set(_allDefinitions "")
    set(_allLinkFlags "")
    set(_allPchDirs "")
    set(_cpackName modules)
    set(_pchDisabledForThisModule FALSE)
    string(TOUPPER ${project_name} u_project_name)
    if(u_project_name MATCHES "QT+")
        set(_cpackName qt_modules)
    endif()
endmacro()

#--------------------------------------------------------------------
# Creates a inviwo module
# Defines:  (example project_name = OpenGL)
#    _packageName           -> InviwoOpenGLModule
#    _preModuleDependencies -> ""
#   PARENT_SCOPE:
#   IVW_MODULE_CLASS        -> OpenGL
#   IVW_MODULE_PACKAGE_NAME -> InviwoOpenGLModule
macro(ivw_module project_name)
    string(TOLOWER ${project_name} l_project_name)
    ivw_project(${l_project_name})
    set(_packageName Inviwo${project_name}Module)
    set(_preModuleDependencies "")
    set(IVW_MODULE_CLASS ${project_name} PARENT_SCOPE)
    set(IVW_MODULE_PACKAGE_NAME ${_packageName} PARENT_SCOPE)
endmacro()

#--------------------------------------------------------------------
# Creates project with initial variables
macro(ivw_set_cpack_name cpack_name)
    set(_cpackName ${cpack_name})
endmacro()

#--------------------------------------------------------------------
# Add cmake module path
macro(ivw_add_cmake_find_package_path)
    foreach(item ${ARGN})
        set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${item})
    endforeach()
endmacro()

#--------------------------------------------------------------------
# Register the use of modules
macro(ivw_register_use_of_modules)
    foreach(module ${ARGN})
        string(TOUPPER ${module} u_module)
        ivw_add_definition(REG_${u_module})
    endforeach()
endmacro()

#--------------------------------------------------------------------
# Retrieve all modules as a list
function(ivw_retrieve_all_modules module_list)
    set(${module_list} ${ivw_all_registered_modules} PARENT_SCOPE)
endfunction()

#--------------------------------------------------------------------
# Generate header for external modules
function(ivw_generate_module_paths_header)
    set(dirs "")
    foreach(dir ${IVW_ROOT_DIR}/modules ${IVW_EXTERNAL_MODULES})
        if(IS_DIRECTORY ${dir})
            list(APPEND dirs ${dir})
        else()
            ivw_message("Path to external module is not a directory (${dir})")
        endif()
    endforeach()

    list_to_longstringvector(vec ${dirs})
    list(LENGTH IVW_EXTERNAL_MODULES count)
    math(EXPR count "${count}+1")
    set(paths "const std::array<const std::string, ${count}> inviwoModulePaths_ = {${vec}}")

    set(IVW_MODULES_PATHS_ARRAY ${paths})

    configure_file(${IVW_CMAKE_TEMPLATES}/inviwomodulespaths_template.h 
                   ${CMAKE_BINARY_DIR}/modules/_generated/inviwomodulespaths.h @ONLY IMMEDIATE)
endfunction()

#--------------------------------------------------------------------
# Generate a module registration header file (with configure file etc)
function(ivw_private_generate_module_registration_file modules_var)
    set(headers "")
    set(functions "")
    foreach(mod ${${modules_var}})      
        set(header
            "#ifdef REG_${mod}\n"
            "#include <${${mod}_dir}/${${mod}_dir}module.h>\n"
            "#endif\n"
        )
        join(";" "" header ${header})

        ivw_mod_name_to_dir(module_dependencies ${${mod}_dependencies})
        list_to_stringvector(module_depends_vector ${module_dependencies})
        set(factory_object
            "    #ifdef REG_${mod}\n" 
            "    modules.emplace_back(new InviwoModuleFactoryObjectTemplate<${${mod}_class}Module>(\n"
            "        \"${${mod}_class}\",\n"
            "        \"${${mod}_description}\",\n" 
            "        ${module_depends_vector}\n" 
            "        )\n"
            "    )__SEMICOLON__\n"
            "    #endif\n"
            "\n"
        )
        join(";" "" factory_object ${factory_object})

        list(APPEND headers ${header})
        list(APPEND functions ${factory_object})
    endforeach()

    join(";" "" headers ${headers})
    join(";" "" functions ${functions})

    # undo encoding of linebreaks and semicolon in the module description read from file
    # linebreaks are replaced with '\n"'
    string(REPLACE "__LINEBREAK__" "\\n\"\n        \"" functions "${functions}")
    string(REPLACE "__SEMICOLON__" ";" functions "${functions}")

    string(REPLACE ":" ";" MODULE_HEADERS "${headers}")   
    string(REPLACE ":" ";" MODULE_CLASS_FUNCTIONS "${functions}")

    configure_file(${IVW_CMAKE_TEMPLATES}/mod_registration_template.h 
                   ${CMAKE_BINARY_DIR}/modules/_generated/moduleregistration.h @ONLY)
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

function(ivw_private_is_valid_module_dir path dir retval)
    if(IS_DIRECTORY ${module_path}/${dir})
        string(TOLOWER ${dir} test)
        string(REPLACE " " "" ${test} test)
        if(${dir} STREQUAL ${test})
            if(EXISTS ${module_path}/${dir}/CMakeLists.txt)
                ivw_private_get_ivw_module_name(${module_path}/${dir}/CMakeLists.txt name)
                string(TOLOWER ${name} l_name)
                if(${dir} STREQUAL ${l_name})
                    set(${retval} TRUE PARENT_SCOPE)
                    return()
                else()
                    ivw_message(WARNING "Found invalid module \"${dir}\" at \"${module_path}\". "
                        "ivw_module called with \"${name}\" which is different from the directory \"${dir}\""
                        "They should be the same except for casing.")
                endif()
            else()
                ivw_message(WARNING "Found invalid module \"${dir}\" at \"${module_path}\". "
                    "CMakeLists.txt is missing")
            endif()
        else()
            ivw_message(WARNING "Found invalid module dir \"${dir}\" at \"${module_path}\". "
                    "Dir names should be all lowercase and without spaces")
        endif()
    endif()
    set(${retval} FALSE PARENT_SCOPE)
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
    foreach(module_path ${IVW_MODULE_DIR} ${IVW_EXTERNAL_MODULES})
        string(STRIP ${module_path} module_path)
        file(GLOB dirs RELATIVE ${module_path} ${module_path}/[^.]*)
        foreach(dir ${dirs})
            ivw_dir_to_mod_dep(mod ${dir})
            list(FIND modules ${mod} found)
            if(NOT ${found} EQUAL -1)
                ivw_message(WARNING "Module with name ${dir} already added at ${${mod}_path}")
                continue()
            endif()
            ivw_private_is_valid_module_dir(${module_path} ${dir} valid)
            if(${valid})
                ivw_debug_message(STATUS "register module: ${dir}")
                ivw_dir_to_mod_prefix(opt ${dir})           # OpenGL -> IVW_MODULE_OPENGL
                ivw_dir_to_module_taget_name(target ${dir}) # OpenGL -> inviwo-module-opengl
                # Get the classname with the right casing
                ivw_private_get_ivw_module_name(${module_path}/${dir}/CMakeLists.txt name)
                list(APPEND modules ${mod})
                set("${mod}_dir"    "${dir}"                CACHE INTERNAL "Module dir")
                set("${mod}_base"   "${module_path}"        CACHE INTERNAL "Module base")
                set("${mod}_path"   "${module_path}/${dir}" CACHE INTERNAL "Module path")
                set("${mod}_opt"    "${opt}"                CACHE INTERNAL "Module cmake option")
                set("${mod}_target" "${target}"             CACHE INTERNAL "Module target")
                set("${mod}_class"  "${name}"               CACHE INTERNAL "Module class")
                set("${mod}_name"   "Inviwo${name}Module"   CACHE INTERNAL "Module name")

                # Check of there is a depends.cmake
                # Defines dependencies, aliases
                # Save dependencies to INVIWO<NAME>MODULE_dependencies
                # Save aliases to INVIWO<NAME>MODULE_aliases
                if(EXISTS "${${mod}_path}/depends.cmake")
                    set(dependencies "")
                    set(aliases "")
                    include(${${mod}_path}/depends.cmake) 
                    set("${mod}_dependencies" ${dependencies} CACHE INTERNAL "Module dependencies")
                    set("${mod}_aliases" ${aliases} CACHE INTERNAL "Module aliases")
                    unset(dependencies)
                    unset(aliases)
                endif()

                # Check if there is a readme.md of the module. 
                # In that case set to INVIWO<NAME>MODULE_description
                if(EXISTS "${${mod}_path}/readme.md")
                    file(READ "${${mod}_path}/readme.md" description)
                    # encode linebreaks, i.e. '\n', and semicolon in description for
                    # proper handling in CMAKE
                    encodeLineBreaks(cdescription ${description})
                    set("${mod}_description" ${cdescription} CACHE INTERNAL "Module description")
                endif()
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
        set("${mod}_dependencies" ${new_dependencies} CACHE INTERNAL "Module dependencies")
    endforeach()

    # Filter out inviwo dependencies
    foreach(mod ${modules})
        set(ivw_dependencies "")
        foreach(dependency ${${mod}_dependencies})
            ivw_mod_name_to_mod_dep(dep ${dependency})
            list(FIND modules ${dep} found)
            if(NOT ${found} EQUAL -1) # This is a dependency to a inviwo module
                list(APPEND ivw_dependencies ${dep})
            endif()
        endforeach()
        set("${mod}_ivw_dependencies" ${ivw_dependencies} CACHE INTERNAL "Module inviwo module dependencies")
    endforeach()

    # Sort modules by dependencies
    ivw_topological_sort(modules _ivw_dependencies sorted_modules)
    #ivw_print_list(sorted_modules)

    # enable depencenies
    ivw_reverse_list_copy(sorted_modules rev_sorted_modules)
    foreach(mod ${rev_sorted_modules})
        if(${${mod}_opt})
            foreach(dep ${${mod}_ivw_dependencies})
                if(NOT ${${dep}_opt})
                    ivw_add_module_option_to_cache(${${dep}_dir} ON TRUE)
                    ivw_message(STATUS "${${dep}_opt} was set to build, "
                        "due to dependency towards ${${mod}_opt}")
                endif()
            endforeach()
        endif()
    endforeach()

    # Add enabled modules in sorted order
    set(ivw_module_classes "")
    set(ivw_module_names "")

    set(IVW_MODULE_PACKAGE_NAMES "")
    set(IVW_MODULE_CLASS "")
    set(IVW_MODULE_PACKAGE_NAME "")

    foreach(mod ${sorted_modules})
        if(${${mod}_opt})
            add_subdirectory(${${mod}_path} ${IVW_BINARY_DIR}/modules/${${mod}_dir})
            if(NOT "${${mod}_class}" STREQUAL "${IVW_MODULE_CLASS}")
                ivw_message(WARNING 
                    "Missmatched module class names \"${${mod}_class}\" vs \"${IVW_MODULE_CLASS}\"")
            endif()
            list(APPEND ivw_module_names ${${mod}_name})
            list(APPEND ivw_module_classes ${${mod}_class})
            list(APPEND IVW_MODULE_PACKAGE_NAMES ${IVW_MODULE_PACKAGE_NAME})
        endif()
    endforeach()

    list(REMOVE_DUPLICATES ivw_module_classes)

    # Save list of modules
    set(ivw_all_registered_modules ${ivw_module_names} CACHE INTERNAL "All registered inviwo modules")

    # Generate module registration file
    ivw_private_generate_module_registration_file(sorted_modules)

    # Save information for python tools.
    ivw_private_create_pyconfig("${IVW_MODULE_DIR};${IVW_EXTERNAL_MODULES}" "${ivw_module_classes}")

    set(${retval} ${sorted_modules} PARENT_SCOPE)
endfunction()


#--------------------------------------------------------------------
# Add all minimal applications in folder
macro(ivw_add_minimal_applications)
    file(GLOB sub-dir RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/minimals ${CMAKE_CURRENT_SOURCE_DIR}/minimals/*)
    list(REMOVE_ITEM sub-dir .svn)
    set(sorted_dirs ${sub-dir})
    foreach(dir ${sub-dir})
        if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/minimals/${dir})
            string(TOUPPER ${dir} u_dir)
            option(IVW_TINY_${u_dir}_APPLICATION "Build Inviwo Tiny ${u_dir} Application" OFF)
            if(NOT ${u_dir} STREQUAL "QT")
                ivw_private_build_module_dependency(${u_dir} IVW_TINY_${u_dir}_APPLICATION)
            endif()
            if(IVW_TINY_${u_dir}_APPLICATION)
                add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/minimals/${dir})
            endif()
        endif()
    endforeach()
endmacro()

#--------------------------------------------------------------------
# Set module build option to true if the owner is built
function(ivw_private_build_module_dependency the_module the_owner)
    ivw_dir_to_mod_prefix(mod_name ${the_module})
    first_case_upper(dir_name_cap ${the_module})
    if(${the_owner} AND NOT ${mod_name})
        ivw_add_module_option_to_cache(${the_module} ON TRUE)
        ivw_message(STATUS "${mod_name} was set to build, due to dependency towards ${the_owner}")
    endif()
endfunction()

#--------------------------------------------------------------------
# Add all external modules specified in cmake string IVW_EXTERNAL_MODULES
macro(ivw_add_external_projects)
    foreach(project_root_path ${IVW_EXTERNAL_PROJECTS})
        string(STRIP ${project_root_path} project_root_path)
        get_filename_component(FOLDER_NAME ${project_root_path} NAME)
        add_subdirectory(${project_root_path} ${CMAKE_CURRENT_BINARY_DIR}/ext_${FOLDER_NAME})
    endforeach()
endmacro()

#--------------------------------------------------------------------
# Set module build option to true
function(ivw_enable_module the_module)
    ivw_add_module_option_to_cache(${the_module} ON FALSE)
endfunction()

#--------------------------------------------------------------------
# Add a library dependency to module. call before ivw_create_module
macro(add_dependency_libs_to_module)
    list(APPEND _preModuleDependencies "${ARGN}")
endmacro()

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
function(ivw_folder project_name folder_name)
    set_target_properties(${project_name} PROPERTIES FOLDER ${folder_name})
endfunction()

#--------------------------------------------------------------------
# Specify console as target
function(ivw_define_standard_properties project_name)
    if(NOT MSVC)
        set_property(TARGET ${project_name} PROPERTY CXX_STANDARD 11)
        set_property(TARGET ${project_name} PROPERTY CXX_STANDARD_REQUIRED ON)
    endif()

    # Specify warnings
    if(APPLE)
        #https://developer.apple.com/library/mac/documentation/DeveloperTools/Reference/XcodeBuildSettingRef/1-Build_Setting_Reference/build_setting_ref.html
        set_property(TARGET ${project_name}  PROPERTY XCODE_ATTRIBUTE_GCC_WARN_NON_VIRTUAL_DESTRUCTOR YES)
        set_property(TARGET ${project_name}  PROPERTY XCODE_ATTRIBUTE_GCC_WARN_UNUSED_FUNCTION YES)
        set_property(TARGET ${project_name}  PROPERTY XCODE_ATTRIBUTE_GCC_WARN_UNUSED_VARIABLE YES)
        set_property(TARGET ${project_name}  PROPERTY XCODE_ATTRIBUTE_GCC_WARN_HIDDEN_VIRTUAL_FUNCTIONS YES)
        set_property(TARGET ${project_name}  PROPERTY XCODE_ATTRIBUTE_GCC_WARN_ABOUT_MISSING_FIELD_INITIALIZERS YES)
        set_property(TARGET ${project_name}  PROPERTY XCODE_ATTRIBUTE_GCC_WARN_ABOUT_RETURN_TYPE YES)
        set_property(TARGET ${project_name}  PROPERTY XCODE_ATTRIBUTE_GCC_WARN_EFFECTIVE_CPLUSPLUS_VIOLATIONS YES)
        set_property(TARGET ${project_name}  PROPERTY XCODE_ATTRIBUTE_GCC_WARN_PEDANTIC YES)
        set_property(TARGET ${project_name}  PROPERTY XCODE_ATTRIBUTE_GCC_WARN_SHADOW YES)
        set_property(TARGET ${project_name}  PROPERTY XCODE_ATTRIBUTE_GCC_WARN_SIGN_COMPARE YES)
        set_property(TARGET ${project_name}  PROPERTY XCODE_ATTRIBUTE_CLANG_WARN_ENUM_CONVERSION YES)
        set_property(TARGET ${project_name}  PROPERTY XCODE_ATTRIBUTE_WARNING_CFLAGS "-Wunreachable-code")
    endif()
endfunction()

#--------------------------------------------------------------------
# Specify console as target
function(ivw_vs_executable_setup project_name)
    if(WIN32)
      if(MSVC)
         set_target_properties(${project_name} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
         set_target_properties(${project_name} PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE")
         set_target_properties(${project_name} PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
         set_target_properties(${project_name} PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "_CONSOLE")
         set_target_properties(${project_name} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:CONSOLE") 
         set_target_properties(${project_name} PROPERTIES MINSIZEREL "/SUBSYSTEM:_CONSOLE")
        endif(MSVC)
    endif(WIN32)
endfunction()

#--------------------------------------------------------------------
# Define standard defintions
macro(ivw_define_standard_definitions project_name target_name)

    # Set the compiler flags
    ivw_to_macro_name(u_project_name ${project_name})
    target_compile_definitions(${target_name} PRIVATE -D${u_project_name}_EXPORTS)
    target_compile_definitions(${target_name} PRIVATE -DGLM_EXPORTS)

    if(WIN32)
        # Large memory support
        if(CMAKE_SIZEOF_VOID_P MATCHES 4) 
            if(NOT CMAKE_EXE_LINKER_FLAGS MATCHES "/LARGEADDRESSAWARE")
                set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE ")
            endif()
            if(NOT CMAKE_SHARED_LINKER_FLAGS MATCHES "/LARGEADDRESSAWARE")
                set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /LARGEADDRESSAWARE")
            endif()
            if(NOT CMAKE_MODULE_LINKER_FLAGS MATCHES "/LARGEADDRESSAWARE")
                set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /LARGEADDRESSAWARE")
            endif()
        endif()
    else()
        target_compile_definitions(${target_name} PRIVATE -DHAVE_CONFIG_H)
    endif()
        
    if(MSVC)
        target_compile_definitions(${target_name} PRIVATE -DUNICODE)
        target_compile_definitions(${target_name} PRIVATE -D_CRT_SECURE_NO_WARNINGS 
                                                          -D_CRT_SECURE_NO_DEPRECATE)
    endif()

    source_group("CMake Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)
endmacro()

#--------------------------------------------------------------------
# Add definition
macro(ivw_add_definition def)
    add_definitions(-D${def})
    list(APPEND _allDefinitions -D${def})
endmacro()

#--------------------------------------------------------------------
# Add definition to list only 
macro(ivw_add_definition_to_list def)
    list(APPEND _allDefinitions -D${def})
endmacro()

#--------------------------------------------------------------------
# Add folder to module pack
macro(ivw_add_to_module_pack folder)
    set(IVW_SHADER_INCLUDE_PATHS "${IVW_SHADER_INCLUDE_PATHS};${folder}" PARENT_SCOPE)
    
    if(IVW_PACKAGE_PROJECT)
        get_filename_component(FOLDER_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
        if(APPLE)
            install(DIRECTORY ${folder}
                     DESTINATION Inviwo.app/Contents/Resources/modules/${FOLDER_NAME}
                     COMPONENT ${_cpackName})
        else()
            install(DIRECTORY ${folder}
                     DESTINATION modules/${FOLDER_NAME}
                     COMPONENT ${_cpackName})
        endif()
    endif()
endmacro()

#--------------------------------------------------------------------
# Creates project module from name
# This it called from the inviwo module CMakeLists.txt 
# that is included from ivw_register_modules. 
macro(ivw_create_module)
    ivw_debug_message(STATUS "create module: ${_projectName}")

    ivw_dir_to_mod_dep(mod ${_projectName})               # opengl -> INVIWOOPENGLMODULE

    set(CMAKE_FILES "")
    if(EXISTS "${${mod}_path}/depends.cmake")
        list(APPEND CMAKE_FILES "${${mod}_path}/depends.cmake")
    endif()
    if(EXISTS "${${mod}_path}/readme.md")
        list(APPEND CMAKE_FILES "${${mod}_path}/readme.md")
    endif()
    source_group("CMake Files" FILES ${CMAKE_FILES})

    # Add module class files
    set(MOD_CLASS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/${_projectName}module.h)
    list(APPEND MOD_CLASS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/${_projectName}module.cpp)
    list(APPEND MOD_CLASS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/${_projectName}moduledefine.h)
      
    # Create library
    add_library(${${mod}_target} ${ARGN} ${MOD_CLASS_FILES} ${CMAKE_FILES})
    
    # Define standard properties
    ivw_define_standard_definitions(${${mod}_opt} ${${mod}_target})
    ivw_define_standard_properties(${${mod}_target})
    
    # Add dependencies
    set(tmpProjectName ${_projectName})
    set(_projectName ${${mod}_target})
    ivw_add_dependency_libraries(${_preModuleDependencies})
    ivw_add_dependencies(InviwoCore)
    # Add dependencies from depends.cmake
    ivw_add_dependencies(${${mod}_dependencies})

    # Optimize compilation with pre-compilied headers based on inviwo-core
    ivw_compile_optimize_inviwo_core()
    
    set(_projectName ${tmpProjectName})
       
    # Make package (for other modules to find)
    ivw_make_package(${_packageName} ${${mod}_target})

    # Add stuff to the installer
    ivw_private_install_module_dirs()

    ivw_make_unittest_target("${_projectName}" "${${mod}_dependencies}")
endmacro()

#--------------------------------------------------------------------
# Make package (with configure file etc)
macro(ivw_make_package package_name project_name)
    ivw_private_install_package(${project_name})

    list(APPEND _allLibsDir "${IVW_LIBRARY_DIR}")
    if(WIN32 AND BUILD_SHARED_LIBS)
        set(PROJECT_LIBRARIES 
            optimized ${IVW_LIBRARY_DIR}/Release/${project_name}.lib 
            debug ${IVW_LIBRARY_DIR}/Debug/${project_name}${CMAKE_DEBUG_POSTFIX}.lib)
    else()
       set(PROJECT_LIBRARIES 
           optimized ${project_name} 
           debug ${project_name}${CMAKE_DEBUG_POSTFIX})
    endif()
    list(APPEND _allLibs ${PROJECT_LIBRARIES})
  
    remove_duplicates(uniqueIncludes ${_allIncludes})
    remove_duplicates(uniqueIncludeDirs ${_allIncludeDirs})
    remove_duplicates(uniqueLibsDir ${_allLibsDir})
    clean_library_list(_allLibs)
    remove_duplicates(uniqueDefs ${_allDefinitions})
    remove_duplicates(uniqueLinkFlags ${_allLinkFlags})
    
    string(TOUPPER ${package_name} u_package_name)
    set(package_name ${u_package_name})
    set(_allIncludes ${uniqueIncludes})
    set(_allIncludeDirs ${uniqueIncludeDirs})
    set(_allLibsDir ${uniqueLibsDir})
    set(_allLibs ${_allLibs})
    set(_allDefinitions ${uniqueDefs})
    set(_allLinkFlags ${uniqueLinkFlags})
    set(_project_name ${project_name})
  
    configure_file(${IVW_CMAKE_TEMPLATES}/mod_package_template.cmake 
                   ${IVW_CMAKE_BINARY_MODULE_DIR}/Find${package_name}.cmake @ONLY IMMEDIATE)
endmacro()


#--------------------------------------------------------------------
# Install files
function(ivw_private_install_package project_name)
   # Add to package
   if(IVW_PACKAGE_PROJECT AND BUILD_SHARED_LIBS)  
        if(WIN32)
           install(TARGETS ${project_name}
                    RUNTIME DESTINATION bin
                    COMPONENT ${_cpackName})
        
        elseif(APPLE)
            install(TARGETS ${project_name}
                    RUNTIME DESTINATION bin
                    BUNDLE DESTINATION .
                    ARCHIVE DESTINATION Inviwo.app/Contents/MacOS
                    LIBRARY DESTINATION Inviwo.app/Contents/MacOS
                    COMPONENT ${_cpackName})
        else()
            install(TARGETS ${project_name}
                    RUNTIME DESTINATION bin
                    BUNDLE DESTINATION bin
                    ARCHIVE DESTINATION lib
                    LIBRARY DESTINATION lib
                    COMPONENT ${_cpackName})
        endif()
    endif()
endfunction()

function(ivw_private_install_module_dirs)
    if(IVW_PACKAGE_PROJECT) 
        get_filename_component(FOLDER_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
        foreach(folder ${CMAKE_CURRENT_SOURCE_DIR}/data)
            if(EXISTS ${folder})
                if(APPLE)
                    install(DIRECTORY ${folder}
                             DESTINATION Inviwo.app/Contents/Resources/modules/${FOLDER_NAME}
                             COMPONENT ${_cpackName})
                else()
                    install(DIRECTORY ${folder}
                             DESTINATION modules/${FOLDER_NAME}
                             COMPONENT ${_cpackName})
                endif()
            endif()
        endforeach()
    endif()
endfunction()

#--------------------------------------------------------------------
# Add includes
macro(ivw_include_directories)
      # Set includes
      include_directories("${ARGN}")
      # Append includes to project list
      list(APPEND _allIncludeDirs ${ARGN})
endmacro()

#--------------------------------------------------------------------
# Add includes
macro(ivw_link_directories)
    # Set includes
    link_directories("${ARGN}")
    # Append includes to project list
    list(APPEND _allLibsDir ${ARGN})
endmacro()

#--------------------------------------------------------------------
# Add includes
macro(ivw_add_link_flags)
    # Set link flags
    set_target_properties(${project_name} PROPERTIES LINK_FLAGS "${ARGN}")
    # Append includes to project list
    list(APPEND _allLinkFlags "\"${ARGN}\"")
endmacro()

#--------------------------------------------------------------------
# adds link_directories for the supplies dependencies
macro(ivw_add_dependency_directories)
    foreach (package ${ARGN})
        # Locate libraries
        find_package(${package} QUIET REQUIRED)
        ivw_find_package_name(${package} u_package)
      
        # Append library directories to project list
        set(uniqueNewLibDirs ${${u_package}_LIBRARY_DIR})
        remove_from_list(uniqueNewLibDirs "${${u_package}_LIBRARY_DIR}" ${_allLibsDir})
        set(${u_package}_LIBRARY_DIR ${uniqueNewLibDirs})
        list(APPEND _allLibsDir ${${u_package}_LIBRARY_DIR})

        # Set directory links
        link_directories(${${u_package}_LIBRARY_DIR})
    endforeach()
endmacro()

#--------------------------------------------------------------------
# internal function call by ivw_create_module
# call add_dependency_libs_to_module instead before, ivw_crete_module
# Adds dependancy and includes package variables to the project

macro(ivw_add_dependency_libraries)
    if(${ARGC} GREATER 0)
        set(uniqueNewLibs ${ARGN})
        remove_library_list(uniqueNewLibs "${ARGN}" ${_allLibs})
        set(${ARGN} ${uniqueNewLibs})
        target_link_libraries(${_projectName} ${ARGN})
        list (APPEND _allLibs ${ARGN})
    endif()
endmacro()

#--------------------------------------------------------------------
# Adds dependency and includes package variables to the project
# 
# Defines: for example the OpenGL module
# INVIWOOPENGLMODULE_DEFINITIONS=
# INVIWOOPENGLMODULE_FOUND=
# INVIWOOPENGLMODULE_INCLUDE_DIR=
# INVIWOOPENGLMODULE_LIBRARIES=
# INVIWOOPENGLMODULE_LIBRARY_DIR=
# INVIWOOPENGLMODULE_LINK_FLAGS=
# INVIWOOPENGLMODULE_USE_FILE=
# 
# Appends to globals:
# _allIncludes
# _allLibsDir
# _allLibs
# _allDefinitions
# _allLinkFlags
# _allIncludeDirs
# 
macro(ivw_add_dependencies)
    foreach (package ${ARGN})
        # Locate libraries
        find_package(${package} QUIET REQUIRED)

        ivw_find_package_name(${package} u_package)
 
        # Set includes and append to list
        if(DEFINED ${u_package}_USE_FILE)
            if(NOT "${${u_package}_USE_FILE}" STREQUAL "")
                include(${${u_package}_USE_FILE})
                list(APPEND _allIncludes \"${${u_package}_USE_FILE}\")
            endif()
        endif()
           
        # Append library directories to project list
        set(uniqueNewLibDirs ${${u_package}_LIBRARY_DIR})
        remove_from_list(uniqueNewLibDirs "${${u_package}_LIBRARY_DIR}" ${_allLibsDir})
        list(APPEND _allLibsDir ${${u_package}_LIBRARY_DIR})
        
        foreach (libDir ${${u_package}_LIBRARY_DIRS})
            remove_from_list(uniqueNewLibDirs ${libDir} ${_allLibsDir})
            list(APPEND _allLibsDir ${libDir})
        endforeach()
        
        set(${u_package}_LIBRARY_DIRS ${uniqueNewLibDirs})
        
        # Append includes to project list
        if(NOT DEFINED ${u_package}_LIBRARIES  AND DEFINED ${u_package}_LIBRARY)
            if(DEFINED ${u_package}_LIBRARY_DEBUG)
                set(${u_package}_LIBRARIES optimized "${${u_package}_LIBRARY}" 
                                           debug "${${u_package}_LIBRARY_DEBUG}")
            else()
                set(${u_package}_LIBRARIES "${${u_package}_LIBRARY}")
            endif()
        endif()
      
        set(uniqueNewLibs ${${u_package}_LIBRARIES})
        remove_library_list(uniqueNewLibs "${${u_package}_LIBRARIES}" ${_allLibs})
        set(${u_package}_LIBRARIES ${uniqueNewLibs})
        list (APPEND _allLibs ${${u_package}_LIBRARIES})
        
        # Append definitions to project list
        set(uniqueNewDefs ${${u_package}_DEFINITIONS})
        remove_from_list(uniqueNewDefs "${${u_package}_DEFINITIONS}" ${_allDefinitions})
        set(${u_package}_DEFINITIONS ${uniqueNewDefs})
        list (APPEND _allDefinitions ${${u_package}_DEFINITIONS})

        # Append link flags to project list
        set(uniqueNewLinkFlags ${${u_package}_LINK_FLAGS})
        remove_from_list(uniqueNewLinkFlags "${${u_package}_LINK_FLAGS}" ${_allLinkFlags})
        set(${u_package}_LINK_FLAGS ${uniqueNewLinkFlags})
        if(NOT "${${u_package}_LINK_FLAGS}" STREQUAL "")
            list (APPEND _allLinkFlags "\"${${u_package}_LINK_FLAGS}\"")
        endif()
    
        # Set includes and append to list
        include_directories(${${u_package}_INCLUDE_DIR})
        list(APPEND _allIncludeDirs ${${u_package}_INCLUDE_DIR})
        include_directories(${${u_package}_INCLUDE_DIRS})
        list(APPEND _allIncludeDirs ${${u_package}_INCLUDE_DIRS})

        # Set directory links
        link_directories(${${u_package}_LIBRARY_DIRS})

        # Set directory links
        add_definitions(${${u_package}_DEFINITIONS})
      
        # Add dependency projects
        if(BUILD_${u_package})
            if(NOT DEFINED ${u_package}_PROJECT)
                set(${u_package}_PROJECT ${package})
            endif()
            add_dependencies(${_projectName} ${${u_package}_PROJECT})
        endif(BUILD_${u_package})
      
        # Link library     
        target_link_libraries(${_projectName} ${${u_package}_LIBRARIES})
      
        # Link flags
        if(NOT "${${u_package}_LINK_FLAGS}" STREQUAL "")
            set_target_properties(${_projectName} PROPERTIES LINK_FLAGS "${${u_package}_LINK_FLAGS}")
        endif()
      
        # Qt5
        set(Qt5DependLibs "")
        foreach (package_lib ${${u_package}_LIBRARIES})
            string(LENGTH "${package_lib}" package_lib_length)
            if(${package_lib_length} GREATER 5)
                string(SUBSTRING "${package_lib}" 0 5 package_lib_start)
                string(SUBSTRING "${package_lib}" 5 -1 package_lib_end)
                if(${package_lib_start} STREQUAL "Qt5::")
                     list(APPEND Qt5DependLibs ${package_lib_end})
                endif()
            endif()
        endforeach()
        remove_duplicates(uniqueQt5DependLibs ${Qt5DependLibs})
        foreach (uniqueQt5Lib ${uniqueQt5DependLibs})
           qt5_use_modules(${_projectName} ${uniqueQt5Lib})
        endforeach()
    endforeach()
endmacro()

#-------------------------------------------------------------------#
#                            QT stuff                               #
#-------------------------------------------------------------------#
# Wrap Qt CPP to create MOC files
macro(ivw_qt_wrap_cpp retval)
    qt5_wrap_cpp(the_list ${ARGN})
    set(${retval} ${the_list})
endmacro()

#--------------------------------------------------------------------
# Set automoc on a target
macro(ivw_qt_automoc project_name)
    set_target_properties(${project_name} PROPERTIES AUTOMOC TRUE)
endmacro()

#--------------------------------------------------------------------
# Define QT definitions
macro(ivw_define_qt_definitions)
    if(WIN32) 
        # Disable some warnings for qt modules.
        add_definitions( "/wd4800" ) # 'type' : forcing value to bool 'true' or 'false'
    endif()

    add_definitions(-DQT_CORE_LIB
                    -DQT_GUI_LIB)
endmacro()

#--------------------------------------------------------------------
# Adds special qt dependency and includes package variables to the project
macro(ivw_qt_add_to_install qtarget ivw_comp)
    if(IVW_PACKAGE_PROJECT)
        find_package(${qtarget} QUIET REQUIRED)
        if(${qtarget}_FOUND)
            if(WIN32)
                set(QTARGET_DIR "${${qtarget}_DIR}/../../../bin")
                install(FILES ${QTARGET_DIR}/${qtarget}${CMAKE_DEBUG_POSTFIX}.dll 
                        DESTINATION bin 
                        COMPONENT ${ivw_comp} 
                        CONFIGURATIONS Debug)
                install(FILES ${QTARGET_DIR}/${qtarget}.dll 
                        DESTINATION bin 
                        COMPONENT ${ivw_comp} 
                        CONFIGURATIONS Release)
            elseif(APPLE)
                foreach(plugin ${${qtarget}_PLUGINS})
                    get_target_property(_loc ${plugin} LOCATION)
                    get_filename_component(_path ${_loc} PATH)
                    get_filename_component(_dirname ${_path} NAME)
                    install(FILES ${_loc} 
                            DESTINATION Inviwo.app/Contents/plugins/${_dirname} 
                            COMPONENT ${ivw_comp})
                endforeach()
            endif()
        endif()
    endif()
endmacro()

#-------------------------------------------------------------------#
#                        Precompile headers                         #
#-------------------------------------------------------------------#
# Add directory to precompilied headers
macro(ivw_add_pch_path)
    list(APPEND _allPchDirs ${ARGN})
endmacro()

#--------------------------------------------------------------------
# Creates project with initial variables
macro(ivw_set_pch_disabled_for_module)
    set(_pchDisabledForThisModule TRUE)
endmacro()

#--------------------------------------------------------------------
# Set header ignore paths for cotire
macro(ivw_cotire_ignore)
    get_target_property(COTIRE_PREFIX_HEADER_IGNORE_PATH ${_projectName} COTIRE_PREFIX_HEADER_IGNORE_PATH)
    if(NOT COTIRE_PREFIX_HEADER_IGNORE_PATH)
        set(COTIRE_PREFIX_HEADER_IGNORE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
    
    list(APPEND COTIRE_PREFIX_HEADER_IGNORE_PATH $IVW_COTIRE_EXCLUDES})
    list(REMOVE_DUPLICATES COTIRE_PREFIX_HEADER_IGNORE_PATH)

    set_target_properties(${_projectName} PROPERTIES COTIRE_PREFIX_HEADER_IGNORE_PATH "${COTIRE_PREFIX_HEADER_IGNORE_PATH}")  
endmacro()

#--------------------------------------------------------------------
# Optimize compilation with pre-compilied headers from inviwo core
macro(ivw_compile_optimize_inviwo_core)
    if(PRECOMPILED_HEADERS)
        if(_pchDisabledForThisModule)
            set_target_properties(${_projectName} PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)
        endif()

        ivw_cotire_ignore()
        set_target_properties(${_projectName} PROPERTIES COTIRE_ADD_UNITY_BUILD FALSE)
        get_target_property(_prefixHeader inviwo-core COTIRE_CXX_PREFIX_HEADER)
        set_target_properties(${_projectName} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "${_prefixHeader}")
        cotire(${_projectName})
    endif()
endmacro()

#--------------------------------------------------------------------
# Optimize compilation with pre-compilied headers
macro(ivw_compile_optimize)
    if(PRECOMPILED_HEADERS)
        ivw_cotire_ignore()
        set_target_properties(${_projectName} PROPERTIES COTIRE_ADD_UNITY_BUILD FALSE)
        list(APPEND _allPchDirs ${IVW_EXTENSIONS_DIR})
        set_target_properties(${_projectName} PROPERTIES COTIRE_PREFIX_HEADER_INCLUDE_PATH "${_allPchDirs}")
        cotire(${_projectName})
    endif()
endmacro()