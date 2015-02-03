 #################################################################################
 #
 # Inviwo - Interactive Visualization Workshop
 # Version 0.9
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

include(${CMAKE_CURRENT_LIST_DIR}/clean_library_list.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cotire.cmake)

mark_as_advanced(COTIRE_ADDITIONAL_PREFIX_HEADER_IGNORE_EXTENSIONS 
COTIRE_ADDITIONAL_PREFIX_HEADER_IGNORE_PATH 
COTIRE_DEBUG 
COTIRE_MAXIMUM_NUMBER_OF_UNITY_INCLUDES 
COTIRE_MINIMUM_NUMBER_OF_TARGET_SOURCES
COTIRE_UNITY_SOURCE_EXCLUDE_EXTENSIONS
COTIRE_VERBOSE)

#--------------------------------------------------------------------
# Creates project with initial variables
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
# Creates project with initial variables
macro(ivw_set_cpack_name cpack_name)
  set(_cpackName ${cpack_name})
endmacro()

#--------------------------------------------------------------------
# Add cmake moudle path
macro(ivw_add_cmake_find_package_path)
    foreach(item ${ARGN})
		set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${item})
    endforeach()
endmacro()

#--------------------------------------------------------------------
# Add unittests
macro(ivw_add_unittest)
	if(IVW_MODULE_UNITTESTS)
    foreach(item ${ARGN})
        file(APPEND ${CMAKE_BINARY_DIR}/modules/_generated/unittests_temp.h "\"${item}\"\n")
    endforeach()
	endif()
endmacro()

#--------------------------------------------------------------------
# Convert module name to directory name
macro(first_case_upper retval in_value)
    string(TOLOWER ${in_value} value)
    string(SUBSTRING ${value} 0 1 first_letter)
    string(TOUPPER ${first_letter} first_letter)
    string(REGEX REPLACE "^.(.*)" "${first_letter}\\1" result "${value}")
    set(${retval} ${result})
endmacro()

#--------------------------------------------------------------------
# Join list
macro(join sep glue output)
  string (REGEX REPLACE "([^\\]|^)${sep}" "\\1${glue}" _TMP_STR "${ARGN}")
  string (REGEX REPLACE "[\\](.)" "\\1" _TMP_STR "${_TMP_STR}")
  set(${output} "${_TMP_STR}")
endmacro()

#--------------------------------------------------------------------
# Convert module prefix to directory name
macro(ivw_dir_to_mod_prefix retval)
    set(the_list "")
    foreach(item ${ARGN})
        string(TOUPPER ${item} u_item)
        list(APPEND the_list IVW_MODULE_${u_item})
    endforeach()
    set(${retval} ${the_list})
endmacro()

#--------------------------------------------------------------------
# Convert name to upper (if not certain string)
macro(ivw_depend_name retval)
    set(result ${ARGN})
    if(DESIRED_QT_VERSION MATCHES 5)
        string(REGEX MATCH "(^Qt5)" found_item ${result})
        if(NOT found_item)
            string(TOUPPER ${result} result)
        endif()
    else()
        string(TOUPPER ${result} result)
    endif()
    set(${retval} ${result})
endmacro()

#--------------------------------------------------------------------
# Wrap Qt CPP to create MOC files
macro(ivw_qt_wrap_cpp retval)
    set(the_list "")
if(WIN32)
    set(output_dir ${CMAKE_CURRENT_BINARY_DIR}/_moc)
    set(the_arg_list ${ARGN})
    list(GET the_arg_list 0 first_item)
    if(${first_item} MATCHES "/include/inviwo/")
        set(output_dir ${IVW_BINARY_DIR}/src/_moc)
    elseif(${first_item} MATCHES "/modules/")
        set(output_dir ${IVW_BINARY_DIR}/modules/_moc)
    endif()
    if(DESIRED_QT_VERSION MATCHES 5)
        foreach(item ${the_arg_list})
            get_filename_component(_moc_outfile_name "${item}" NAME_WE)
            qt5_generate_moc(${item} ${output_dir}/moc_${_moc_outfile_name}.cxx)
            list(APPEND the_list ${output_dir}/moc_${_moc_outfile_name}.cxx)
        endforeach()
    else()
        foreach(item ${the_arg_list})
            get_filename_component(_moc_outfile_name "${item}" NAME_WE)
            qt4_generate_moc(${item} ${output_dir}/moc_${_moc_outfile_name}.cxx)
            list(APPEND the_list ${output_dir}/${_moc_outfile_name})
        endforeach()
    endif()
else()
    if(DESIRED_QT_VERSION MATCHES 5)
       qt5_wrap_cpp(the_list ${ARGN})
    else()
       qt4_wrap_cpp(the_list ${ARGN})
    endif()
endif()
    set(${retval} ${the_list})
endmacro()

#--------------------------------------------------------------------
# Convert module prefix to directory name
macro(ivw_mod_prefix_to_dir retval)
    set(the_list "")
    foreach(item ${ARGN})
        string(REGEX MATCH "(^IVW_MODULE_$)" found_item ${item})
        if(found_item)
            string(REGEX REPLACE "(^IVW_MODULE_$)" "" new_item ${item})
            string(TOLOWER ${new_item} l_new_item)
            list(APPEND the_list ${l_new_item})
        endif()
    endforeach()
    set(${retval} ${the_list})
endmacro()

#--------------------------------------------------------------------
# Convert module name to directory name
macro(ivw_mod_name_to_dir retval)
    set(the_list "")
    foreach(item ${ARGN})
        string(REGEX MATCH "(^Inviwo.*.Module$)" found_item ${item})
        if(found_item)
            string(REGEX REPLACE "(^Inviwo)|(Module$)" "" new_item ${item})
            string(TOLOWER ${new_item} l_new_item)
            list(APPEND the_list ${l_new_item})
        endif()
    endforeach()
    set(${retval} ${the_list})
endmacro()

#--------------------------------------------------------------------
# Creates module
macro(ivw_module project_name)
  string(TOLOWER ${project_name} l_project_name)
  ivw_project(${l_project_name})
  set(_packageName Inviwo${project_name}Module)
  set(_preModuleDependencies "")
  set(IVW_MODULE_CLASS ${project_name} PARENT_SCOPE)
  set(IVW_MODULE_CLASS_PATH "${l_project_name}/${l_project_name}module" PARENT_SCOPE)
  set(IVW_MODULE_PACKAGE_NAME ${_packageName} PARENT_SCOPE)
endmacro()

#--------------------------------------------------------------------
# List subdirectories
macro(list_subdirectories retval curdir return_relative)
   file(GLOB sub-dir RELATIVE ${curdir} ${curdir}/[^.svn]*)
   set(list_of_dirs "")
   foreach(dir ${sub-dir})
     if(IS_DIRECTORY ${curdir}/${dir})
       if (${return_relative})
         set(list_of_dirs ${list_of_dirs} ${dir})
       else()
         set(list_of_dirs ${list_of_dirs} ${curdir}/${dir})
       endif()
     endif()
   endforeach()
   set(${retval} ${list_of_dirs})
endmacro()

#--------------------------------------------------------------------
# Clean duplicates from list subdirectories
macro(remove_duplicates retval)
    set(list_of_dirs ${ARGN})
	if(list_of_dirs)
        list(REMOVE_DUPLICATES list_of_dirs)
    endif()
    set(${retval} ${list_of_dirs})
endmacro()

#--------------------------------------------------------------------
# Remove entries in one list from another list
macro(remove_from_list retval thelist)
    set(new_items ${thelist})
    set(old_items ${ARGN})
    if(old_items AND new_items)
        foreach(item ${old_items})
            list(REMOVE_ITEM new_items ${item})
        endforeach()
    endif()
    set(${retval} ${new_items})
endmacro()

#--------------------------------------------------------------------
# Collect and sort list variables
macro(list_filter_remove retval pattern)
    foreach(item ${ARGN})
        string(REGEX MATCH "^#" found_item ${item})
        if(NOT found_item)
            #list(REMOVE_ITEM ${ARGN} ${item})
        endif()
    endforeach()
endmacro()

#--------------------------------------------------------------------
# Retrieve all modules as a list
macro(ivw_retrieve_all_modules module_list)
    if(EXISTS "${CMAKE_BINARY_DIR}/modules/_generated/modules.cmake")
        include(${CMAKE_BINARY_DIR}/modules/_generated/modules.cmake)
        foreach(exclude_module ${ARGN})
            list(REMOVE_ITEM module_packages ${exclude_module})
        endforeach()
        set(${module_list} ${module_packages})
    endif()
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
# Generate a list of all available module packages
macro(create_module_package_list)
    set(ALL_MODULE_PACKAGES "")
    foreach(module ${ARGN})
        list(APPEND ALL_MODULE_PACKAGES Inviwo${module}Module)
    endforeach()
    configure_file(${IVW_CMAKE_SOURCE_MODULE_DIR}/modules_template.cmake ${CMAKE_BINARY_DIR}/modules/_generated/modules.cmake @ONLY)
endmacro()

#--------------------------------------------------------------------
# Generate header for external modules
macro(generate_external_module_header)
    set(IVW_EXTERNAL_MODULES_PATH_COUNT "")
    set(IVW_EXTERNAL_MODULES_PATHS_ARRAY "")
    if(DEFINED IVW_EXTERNAL_MODULES)
        list(LENGTH IVW_EXTERNAL_MODULES modlen)
        if(NOT ${modlen} EQUAL 0)
            set(paths "const std::string externalModulePaths_[] = {:")
            set(first "0")
            set(count 0)
            foreach(dir ${IVW_EXTERNAL_MODULES})
                if(IS_DIRECTORY ${dir})
                    if(NOT ${first} EQUAL "0")
                        list(APPEND paths ", :")
                    endif()
                    set(first "1")
                    list(APPEND paths "\"${dir}\":")
                    MATH(EXPR count "${count}+1")
                else()
                    MESSAGE(FATAL_ERROR "Path to external module is not a directory (${dir})")
                endif()
            endforeach()
            list(APPEND paths "}; //Paths")
            set(IVW_EXTERNAL_MODULES_PATH_COUNT "#define IVW_EXTERNAL_MODULES_PATH_COUNT ${count}")
            set(paths ${paths})
            join(":;" "" IVW_EXTERNAL_MODULES_PATHS_ARRAY ${paths})
         endif()
    endif()
    configure_file(${IVW_CMAKE_SOURCE_MODULE_DIR}/mod_external_template.h ${CMAKE_BINARY_DIR}/modules/_generated/pathsexternalmodules.h @ONLY IMMEDIATE)
endmacro()

#--------------------------------------------------------------------
# Generate a module registration header file (with configure file etc)
macro(generate_module_registration_file module_classes modules_class_paths)
    set(module_classes ${module_classes})
    set(modules_class_paths ${modules_class_paths})
    list(LENGTH module_classes len1)
    math(EXPR len0 "${len1} - 1")
    set(headers "")
    set(functions "")
    foreach(val RANGE ${len0})
        list(GET module_classes ${val} current_name)
        string(TOUPPER ${current_name} u_current_name)
        list(GET modules_class_paths ${val} current_path)
        #Apperance: #include "modules/base/basemodule.h" 
        #Apperance: (*app).registerModule(new BaseModule());
        list(APPEND headers "#ifdef REG_INVIWO${u_current_name}MODULE")
        list(APPEND headers "#include <${current_path}.h>")
        list(APPEND headers "#endif")
        list(APPEND functions "#ifdef REG_INVIWO${u_current_name}MODULE:")
        list(APPEND functions "(*app).registerModule(new ${current_name}Module());:")
        list(APPEND functions "#endif:")
    endforeach()
    list(APPEND functions "}")
    set(headers ${headers})
    set(functions ${functions})
    join(";" "\n" MODULE_HEADERS ${headers})
    join(":;" "\n" MODULE_CLASS_FUNCTIONS ${functions})
    configure_file(${IVW_CMAKE_SOURCE_MODULE_DIR}/mod_registration_template.h ${CMAKE_BINARY_DIR}/modules/_generated/moduleregistration.h @ONLY)
endmacro()

#--------------------------------------------------------------------
# Create CMAKE file for pre-process 
macro(ivw_generate_shader_resource parent_path)
    set(output "include(${IVW_ROOT_DIR}/cmake/txt2h.cmake)\n")
	set(shaders "")
	foreach(current_path ${ARGN})
		file(RELATIVE_PATH filepath0 ${parent_path} ${current_path})
		string(REPLACE "/" "_" filepath1 ${filepath0})
		string(REPLACE "." "_" outname ${filepath1})
		#get_filename_component(filename ${current_path} NAME_WE)
		#get_filename_component(extension ${current_path} EXT)
		#string(REPLACE "." "" extension_no_point ${extension})
		#set(outname "${filename}_${extension_no_point}")
		set(outname_with_dollar "\${${outname}}")
		
		set(shaders "${shaders} ${outname}")
				
		#Add: file(READ outname variable)
		set(output "${output}file(READ ${current_path} ${outname})\n")
		
		set(output "${output}string(REPLACE \"\;\" \"?????\" ${outname} \"${outname_with_dollar}\")\n")
		
		set(output "${output}ivw_text_to_header(${outname} ${outname}_header ${outname_with_dollar})\n")
		set(outname_header_with_dollar "\${${outname}_header}")
		
		set(output "${output}string(REPLACE \"?????\" \"\;\" ${outname}_header \"${outname_header_with_dollar}\")\n")
		
		#Add: file(WRITE outname variable)
		#string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR} "" parent_relative_path ${parent_path})
		#string(REPLACE "." "_" current_relative_path_underscore ${current_relative_path})
		set(output "${output}file(WRITE ${CMAKE_BINARY_DIR}/modules/_generated/modules/${_projectName}/glsl/${outname}.h \"${outname_header_with_dollar}\")\n")
    endforeach()
	string(STRIP ${shaders} shaders)
	string(TOUPPER ${_projectName} u_project_name)
	string(TOLOWER ${_projectName} l_project_name)
	add_definitions(-D${u_project_name}_INCLUDE_SHADER_RESOURCES)
	set(output "${output}ivw_create_shader_resource_header(${_projectName} shader_resource ${shaders})\n")
	set(output "${output}file(WRITE ${CMAKE_BINARY_DIR}/modules/_generated/modules/${_projectName}/shader_resources.h \${shader_resource})\n")
	file(WRITE ${CMAKE_BINARY_DIR}/modules/${_projectName}/create_shader_resource.cmake ${output})
	add_custom_command(TARGET inviwo-module-${_projectName} PRE_BUILD COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/modules/${_projectName}/create_shader_resource.cmake)
endmacro()

#--------------------------------------------------------------------
# Generate module options (which was not specifed before) and,
# Sort directories based on dependencies inside directories
macro(generate_unset_mod_options_and_depend_sort module_root_path retval)
    file(GLOB sub-dir RELATIVE ${module_root_path} ${module_root_path}/*)
    list(REMOVE_ITEM sub-dir .svn)
	set(sorted_dirs ${sub-dir})
    foreach(dir ${sub-dir})
        if(IS_DIRECTORY ${module_root_path}/${dir})
            if(EXISTS "${module_root_path}/${dir}/depends.cmake")
                include(${module_root_path}/${dir}/depends.cmake)
                foreach(dependency ${dependencies})
                    list(FIND IVW_MODULE_PACKAGE_NAMES ${dependency} module_index)
                    if(NOT module_index EQUAL -1)
                       list(REMOVE_ITEM dependencies ${dependency})
                    endif()
                endforeach()
                ivw_mod_name_to_dir(depend_folders ${dependencies})
                list(APPEND depend_folders ${dir})
                list(FIND sorted_dirs ${dir} dir_index)
                list(INSERT sorted_dirs ${dir_index} ${depend_folders})
                list(REMOVE_DUPLICATES sorted_dirs)
            endif()
            ivw_dir_to_mod_prefix(mod_name ${dir})
            if(NOT DEFINED ${mod_name})
                first_case_upper(dir_name_cap ${dir})
                option(${mod_name} "Build ${dir_name_cap} Module" OFF)
            endif()
         else()
                list(REMOVE_ITEM sorted_dirs ${dir})
        endif()
    endforeach()
    set(${retval} ${sorted_dirs})
endmacro()

#--------------------------------------------------------------------
# Begin add modules
macro(begin_add_modules)
    set(IVW_MODULE_CLASSES "")
    set(IVW_MODULE_CLASS_PATHS "")
    set(IVW_MODULE_PACKAGE_NAMES "")
    set(IVW_MODULE_CLASS "")
    set(IVW_MODULE_CLASS_PATH "")
    set(IVW_MODULE_PACKAGE_NAME "")
endmacro()

#--------------------------------------------------------------------
# End add modules
macro(end_add_modules)
	list(REMOVE_DUPLICATES IVW_MODULE_CLASSES)
	list(REMOVE_DUPLICATES IVW_MODULE_CLASS_PATHS)
    #Generate module registration file
    generate_module_registration_file("${IVW_MODULE_CLASSES}" "${IVW_MODULE_CLASS_PATHS}")
    create_module_package_list(${IVW_MODULE_CLASSES})
endmacro()

#--------------------------------------------------------------------
# Add subdirectories of modules based on generated options
macro(add_modules module_root_path)
    foreach(module ${ARGN})
        ivw_dir_to_mod_prefix(mod_name ${module})
        if(${mod_name})
            add_subdirectory(${module_root_path}/${module} ${IVW_BINARY_DIR}/modules/${module})
            list(APPEND IVW_MODULE_CLASSES ${IVW_MODULE_CLASS})
            list(APPEND IVW_MODULE_CLASS_PATHS ${IVW_MODULE_CLASS_PATH})
            list(APPEND IVW_MODULE_PACKAGE_NAMES ${IVW_MODULE_PACKAGE_NAME})
        endif()
    endforeach()
endmacro()

#--------------------------------------------------------------------
# Add all minimal applications in folder
macro(add_minimal_applications)
    file(GLOB sub-dir RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/minimals ${CMAKE_CURRENT_SOURCE_DIR}/minimals/*)
    list(REMOVE_ITEM sub-dir .svn)
	set(sorted_dirs ${sub-dir})
    foreach(dir ${sub-dir})
        if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/minimals/${dir})
            string(TOUPPER ${dir} u_dir)
            option(IVW_TINY_${u_dir}_APPLICATION "Build Inviwo Tiny ${u_dir} Application" OFF)
            if(NOT ${u_dir} STREQUAL "QT")
                build_module_dependency(${u_dir} IVW_TINY_${u_dir}_APPLICATION)
            endif()
            if(IVW_TINY_${u_dir}_APPLICATION)
                add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/minimals/${dir})
            endif()
        endif()
    endforeach()
endmacro()

#--------------------------------------------------------------------
# Add all external modules specified in cmake string IVW_EXTERNAL_MODULES
macro(add_internal_modules)
    #Generate module options
    generate_unset_mod_options_and_depend_sort(${IVW_MODULE_DIR} IVW_SORTED_MODULES)

    #Resolve dependencies for selected modules
    resolve_module_dependencies(${IVW_MODULE_DIR} ${IVW_SORTED_MODULES})

    if(IVW_MODULE_OPENGLQT OR IVW_MODULE_PYTHONQT OR IVW_MODULE_PYTHON3QT)
        # Find the QtWidgets library
        if(DESIRED_QT_VERSION MATCHES 5)
            find_package(Qt5Widgets QUIET REQUIRED)
            find_package(Qt5Help QUIET REQUIRED)     
        else()
            find_package(Qt QUIET REQUIRED)
        endif()
    endif()
	
	if(IVW_MODULE_OPENGLQT)
        set(QT_USE_QTOPENGL TRUE)    
    endif()

    #Add modules based on user config file and dependcy resolve
    add_modules(${IVW_MODULE_DIR} ${IVW_SORTED_MODULES})
endmacro()

#--------------------------------------------------------------------
# Add all external modules specified in cmake string IVW_EXTERNAL_MODULES
macro(add_external_modules)
    foreach(module_root_path ${IVW_EXTERNAL_MODULES})
        #Generate module options
        generate_unset_mod_options_and_depend_sort(${module_root_path} IVW_EXTERNAL_SORTED_MODULES)
        
        if(DEFINED IVW_EXTERNAL_SORTED_MODULES)
            #Resolve dependencies for selected modules
            resolve_module_dependencies(${module_root_path} ${IVW_EXTERNAL_SORTED_MODULES})

            #Add modules based on user config file and dependcy resolve
            add_modules(${module_root_path} ${IVW_EXTERNAL_SORTED_MODULES})
        endif()
    endforeach()
endmacro()

#--------------------------------------------------------------------
# Add all external modules specified in cmake string IVW_EXTERNAL_MODULES
macro(add_external_projects)
    foreach(project_root_path ${IVW_EXTERNAL_PROJECTS})
        get_filename_component(FOLDER_NAME ${project_root_path} NAME)
        add_subdirectory(${project_root_path} ${CMAKE_CURRENT_BINARY_DIR}/ext_${FOLDER_NAME})
    endforeach()
endmacro()

#--------------------------------------------------------------------
# Turn On Dependent Module Options
macro(resolve_module_dependencies module_root_path)   
    #Reverse list (as it is depend sorted) and go over dependencies one more time
    #If build is ON, then switch dependencies ON
    set(dir_list ${ARGN})
    list(REVERSE dir_list)
    foreach(dir ${dir_list})
        ivw_dir_to_mod_prefix(mod_name ${dir})
        if(${mod_name})
            if(EXISTS "${module_root_path}/${dir}/depends.cmake")
                include(${module_root_path}/${dir}/depends.cmake)
                ivw_mod_name_to_dir(depend_folders ${dependencies})
                foreach(depend_folder ${depend_folders})
                    ivw_dir_to_mod_prefix(depend_mod_name ${depend_folder})
                    first_case_upper(depend_name_cap ${depend_folder})
                    if(NOT ${depend_mod_name})
                        set(${depend_mod_name} ON CACHE BOOL "Build ${depend_name_cap} Module" FORCE)
                        ivw_message("${depend_mod_name} was set to build, due to dependency towards ${mod_name}")
                    endif()
                endforeach()
            endif()
        endif()
    endforeach()
endmacro()

#--------------------------------------------------------------------
# Set module build option to true
macro(build_module the_module)
    ivw_dir_to_mod_prefix(mod_name ${the_module})
    first_case_upper(dir_name_cap ${the_module})
    option(${mod_name} "Build ${dir_name_cap} Module" ON)
endmacro()

#--------------------------------------------------------------------
# Set module build option to true if the owner is built
macro(build_module_dependency the_module the_owner)
    ivw_dir_to_mod_prefix(mod_name ${the_module})
    first_case_upper(dir_name_cap ${the_module})
    if(${the_owner} AND NOT ${mod_name})
        set(${mod_name} ON CACHE BOOL "Build ${dir_name_cap} Module" FORCE)
        ivw_message("${mod_name} was set to build, due to dependency towards ${the_owner}")
    endif()
endmacro()

#--------------------------------------------------------------------
# Set module build option to true
macro(hide_module the_module)
    ivw_dir_to_mod_prefix(mod_name ${the_module})
    first_case_upper(dir_name_cap ${the_module})
    option(${mod_name} "Build ${dir_name_cap} Module" OFF)
    mark_as_advanced(FORCE  ${mod_name})
endmacro()

#--------------------------------------------------------------------
# Set module build option to true
macro(add_dependency_libs_to_module)
    list(APPEND _preModuleDependencies ${ARGN})
endmacro()

#--------------------------------------------------------------------
# Creates source group structure recursively
macro(ivw_group group_name)
    foreach(currentSourceFile ${ARGN})
        if(NOT IS_ABSOLUTE ${currentSourceFile})
            set(currentSourceFile ${CMAKE_CURRENT_SOURCE_DIR}/${currentSourceFile})
        endif()
        string(REPLACE "include/inviwo/" "src/" currentSourceFileModified ${currentSourceFile})
        file(RELATIVE_PATH folder ${CMAKE_CURRENT_SOURCE_DIR} ${currentSourceFileModified})
        get_filename_component(filename ${folder} NAME)
        string(REPLACE ${filename} "" folder ${folder})
        if(NOT folder STREQUAL "")
            string(REGEX REPLACE "/+$" "" folderlast ${folder})
            string(REPLACE "/" "\\" folderlast ${folderlast})
            source_group("${group_name}\\${folderlast}" FILES ${currentSourceFile})
        else()
            source_group("${group_name}" FILES ${currentSourceFile})
        endif(NOT folder STREQUAL "")
    endforeach(currentSourceFile ${ARGN})
endmacro()

#--------------------------------------------------------------------
# Creates VS folder structure
macro(ivw_vs_folder project_name folder_name)
    #--------------------------------------------------------------------
    # VS Ignore Libs and folder structure
    if(WIN32)
      if(MSVC)
         set_target_properties(${project_name} PROPERTIES
            LINK_FLAGS_DEBUG
            ${VS_MULTITHREADED_DEBUG_DLL_IGNORE_LIBRARY_FLAGS} FOLDER ${folder_name})
         set_target_properties(${project_name} PROPERTIES
             LINK_FLAGS_RELWITHDEBINFO
             ${VS_MULTITHREADED_RELEASE_DLL_IGNORE_LIBRARY_FLAGS} FOLDER ${folder_name})
         set_target_properties(${project_name} PROPERTIES
             LINK_FLAGS_RELEASE
             ${VS_MULTITHREADED_RELEASE_DLL_IGNORE_LIBRARY_FLAGS} FOLDER ${folder_name})
         set_target_properties(${project_name} PROPERTIES
             LINK_FLAGS_MINSIZEREL
             ${VS_MULTITHREADED_RELEASE_DLL_IGNORE_LIBRARY_FLAGS} FOLDER ${folder_name})
        endif(MSVC)
    endif(WIN32)
endmacro()

#--------------------------------------------------------------------
# Add console to debug build only
macro(ivw_qt_automoc project_name)
    set_target_properties(${project_name} PROPERTIES AUTOMOC TRUE)
endmacro()

#--------------------------------------------------------------------
# Specify console as target
macro(ivw_vs_executable_setup project_name)
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
endmacro()

#--------------------------------------------------------------------
# Define standard defintions
macro(ivw_define_standard_definitions project_name)
    #--------------------------------------------------------------------
    # Set the compiler flags
   string(TOUPPER ${project_name} u_project_name)
    if(WIN32)
      add_definitions(-D${u_project_name}_EXPORTS)
	  add_definitions(-DGLM_EXPORTS)
          
    #--------------------------------------------------------------------          
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

    else(WIN32)
      add_definitions(-DHAVE_CONFIG_H)
    endif(WIN32)
    add_definitions(-DUNICODE)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE)
    
    source_group("CMake Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)
endmacro()

#--------------------------------------------------------------------
# Define QT defintions
macro(ivw_define_qt_definitions)
	add_definitions(-DQT_CORE_LIB
                      -DQT_GUI_LIB)
endmacro()

#--------------------------------------------------------------------
# Add defintion
macro(ivw_add_definition def)
    add_definitions(-D${def})
    list(APPEND _allDefinitions -D${def})
endmacro()

#--------------------------------------------------------------------
# Add defintion to list only 
macro(ivw_add_definition_to_list def)
    list(APPEND _allDefinitions -D${def})
endmacro()

#--------------------------------------------------------------------
# Add folder to module pack
macro(ivw_add_to_module_pack folder)
if(IVW_PACKAGE_PROJECT)
    get_filename_component(FOLDER_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    install(DIRECTORY ${folder}
             DESTINATION modules/${FOLDER_NAME}
             COMPONENT ${_cpackName})
endif()
endmacro()

#--------------------------------------------------------------------
# Creates project module from name 
macro(ivw_create_module)
  ivw_dir_to_mod_prefix(${mod_name} ${_projectName})

  ivw_define_standard_definitions(${mod_name})
  
  set(HAS_DEPEND FALSE)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/depends.cmake")
    set(HAS_DEPEND TRUE)
    set(DEPEND_PATH ${CMAKE_CURRENT_SOURCE_DIR}/depends.cmake)
    source_group("CMake Files" FILES ${DEPEND_PATH})
  endif()
  
  #--------------------------------------------------------------------
  # Add module class files
  set(MOD_CLASS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/${_projectName}module.h)
  list(APPEND MOD_CLASS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/${_projectName}module.cpp)
  list(APPEND MOD_CLASS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/${_projectName}moduledefine.h)
    
  #--------------------------------------------------------------------
  # Create library
  add_library(inviwo-module-${_projectName} ${ARGN} ${MOD_CLASS_FILES} ${DEPEND_PATH})
  
  #--------------------------------------------------------------------
  # Add dependencies
  set(tmpProjectName ${_projectName})
  set(_projectName inviwo-module-${tmpProjectName})
  ivw_add_dependency_libraries(${_preModuleDependencies})
  ivw_add_dependencies(InviwoCore)
  if(HAS_DEPEND)
    include(${DEPEND_PATH})
    # Add dependencies to this list
    ivw_add_dependencies(${dependencies})
  endif()
  
  #--------------------------------------------------------------------
  # Optimize compilation with pre-compilied headers based on inviwo-core
  ivw_compile_optimize_inviwo_core()
  
  set(_projectName ${tmpProjectName})
  
  #--------------------------------------------------------------------
  # Add project to VS module folder
  #ivw_vs_folder(inviwo-module-${_projectName} modules)
  
  #--------------------------------------------------------------------
  # Make package (for other modules to find)
  #string(TOUPPER ${project_name} u_project_name)
  ivw_make_package(${_packageName} inviwo-module-${_projectName})
endmacro()

#--------------------------------------------------------------------
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
# Optimize compilation with pre-compilied headers from inviwo core
macro(ivw_compile_optimize_inviwo_core)
      if(PRECOMPILED_HEADERS)
		if(_pchDisabledForThisModule)
			set_target_properties(${_projectName} PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)
		endif()
		 set_target_properties(${_projectName} PROPERTIES COTIRE_ADD_UNITY_BUILD FALSE)
		 get_target_property(_prefixHeader inviwo-core COTIRE_CXX_PREFIX_HEADER)
		 set_target_properties(${_projectName} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "${_prefixHeader}")
		 cotire(${_projectName})
      endif()
	  # else()
			# get_target_property(_pchFile inviwo-core COTIRE_CXX_PRECOMPILED_HEADER)
			# cotire_setup_pch_file_inclusion(
					# ${_language} ${_target} ${_wholeTarget} "${_prefixFile}" "${_pchFile}" ${_sourceFiles})
			# get_filename_component(pchFileDir ${_pchFile} PATH)
			# file(GLOB PDB_FILES "${pchFileDir}/*.pdb")
			# file(COPY ${PDB_FILES} DESTINATION ${_target})
			# file(GLOB IDB_FILES "${pchFileDir}/*.idb")
			# file(COPY ${IDB_FILES} DESTINATION ${_target})
		# endif()
endmacro()

#--------------------------------------------------------------------
# Optimize compilation with pre-compilied headers
macro(ivw_compile_optimize)
      #set_target_properties(${_projectName} PROPERTIES COTIRE_ENABLE_PRECOMPILED_HEADER FALSE)
      if(PRECOMPILED_HEADERS)
		  set_target_properties(${_projectName} PROPERTIES COTIRE_ADD_UNITY_BUILD FALSE)
		  list(APPEND _allPchDirs ${IVW_EXTENSIONS_DIR})
		  set_target_properties(${_projectName} PROPERTIES COTIRE_PREFIX_HEADER_INCLUDE_PATH "${_allPchDirs}")
		  cotire(${_projectName})
      endif()
      #target_link_libraries(${_projectName}_unity ${_allLibs})
endmacro()

#--------------------------------------------------------------------
# Make package (with configure file etc)
macro(ivw_make_package package_name project_name)
if(IVW_PACKAGE_PROJECT AND BUILD_SHARED_LIBS)  
   #--------------------------------------------------------------------
   # Add to package
if(WIN32)
   install(TARGETS ${project_name}
            RUNTIME DESTINATION bin
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

  #--------------------------------------------------------------------
  # Append headers etc of project to include list
  #set(REST ${ARGN})
  #if(NOT REST)
    #get_filename_component(CURRENT_PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} PATH)
    #list(APPEND _allIncludeDirs ${CURRENT_PARENT_DIR})
  #else()
  #  list(APPEND _allIncludeDirs ${ARGN})
  #endif()
  list(APPEND _allLibsDir "${IVW_LIBRARY_DIR}")
  if(WIN32 AND BUILD_SHARED_LIBS)
	set(PROJECT_LIBRARIES optimized ${IVW_LIBRARY_DIR}/Release/${project_name}.lib debug ${IVW_LIBRARY_DIR}/Debug/${project_name}${CMAKE_DEBUG_POSTFIX}.lib)
  else()
	set(PROJECT_LIBRARIES optimized ${project_name} debug ${project_name}${CMAKE_DEBUG_POSTFIX})
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
  
  configure_file(${IVW_CMAKE_SOURCE_MODULE_DIR}/mod_package_template.cmake ${IVW_CMAKE_BINARY_MODULE_DIR}/Find${package_name}.cmake @ONLY IMMEDIATE)
endmacro()

#--------------------------------------------------------------------
# Add includes
macro(ivw_include_directories)
      #--------------------------------------------------------------------
      # Set includes
      include_directories("${ARGN}")
          
      #--------------------------------------------------------------------
      # Append includes to project list
      list(APPEND _allIncludeDirs ${ARGN})
endmacro()

#--------------------------------------------------------------------
# Add includes
macro(ivw_link_directories)
      #--------------------------------------------------------------------
      # Set includes
      link_directories("${ARGN}")
          
      #--------------------------------------------------------------------
      # Append includes to project list
      list(APPEND _allLibsDir ${ARGN})
endmacro()

#--------------------------------------------------------------------
# Add includes
macro(ivw_add_link_flags)
      #--------------------------------------------------------------------
      # Set link flags
      set_target_properties(${project_name} PROPERTIES LINK_FLAGS "${ARGN}")
         
      #--------------------------------------------------------------------
      # Append includes to project list
      list(APPEND _allLinkFlags "\"${ARGN}\"")
endmacro()

#--------------------------------------------------------------------
# Defines option for module and add subdirectory if such is ON
macro(ivw_add_dependency_directories)
    foreach (package ${ARGN})
      #--------------------------------------------------------------------
      # Locate libraries
      find_package(${package} QUIET REQUIRED)
      
      #--------------------------------------------------------------------
      # Make string upper case
      #string(TOUPPER ${package} u_package)
      ivw_depend_name(u_package ${package})
      
      #--------------------------------------------------------------------
      # Add dependcy package variables to this package if shared build
      #if(NOT BUILD_SHARED_LIBS)
          #--------------------------------------------------------------------
          # Append library directories to project list
          set(uniqueNewLibDirs ${${u_package}_LIBRARY_DIR})
          remove_from_list(uniqueNewLibDirs "${${u_package}_LIBRARY_DIR}" ${_allLibsDir})
          set(${u_package}_LIBRARY_DIR ${uniqueNewLibDirs})
          list(APPEND _allLibsDir ${${u_package}_LIBRARY_DIR})
      #endif()

      #--------------------------------------------------------------------
      # Set directory links
      link_directories(${${u_package}_LIBRARY_DIR})
    endforeach()
endmacro()

#--------------------------------------------------------------------
# Adds dependancy and includes package variables to the project
macro(ivw_add_dependency_libraries)
    foreach (package ${ARGN})
      #--------------------------------------------------------------------
      # Add dependcy package variables to this package if shared build
      #if(NOT BUILD_SHARED_LIBS)         
          #--------------------------------------------------------------------
          # Append includes to project list
          set(uniqueNewLibs ${package})
          remove_library_list(uniqueNewLibs "${package}" ${_allLibs})
          set(${package} ${uniqueNewLibs})
          list (APPEND _allLibs ${package})
      #endif()
    
      #--------------------------------------------------------------------
      # Link library
      target_link_libraries(${_projectName} ${package})
    endforeach()
endmacro()

#--------------------------------------------------------------------
# Adds dependancy and includes package variables to the project
macro(ivw_add_dependencies)
    foreach (package ${ARGN})
      #--------------------------------------------------------------------
      # Locate libraries
      find_package(${package} QUIET REQUIRED)
      
      #--------------------------------------------------------------------
      # Make string upper case
      #string(TOUPPER ${package} u_package)
      ivw_depend_name(u_package ${package})
      
      #--------------------------------------------------------------------
      # Set includes and append to list
      if(DEFINED ${u_package}_USE_FILE)
        if(NOT "${${u_package}_USE_FILE}" STREQUAL "")
            include(${${u_package}_USE_FILE})
            list(APPEND _allIncludes \"${${u_package}_USE_FILE}\")
        endif()
      endif()
           
      #--------------------------------------------------------------------
      # Add dependcy package variables to this package if shared build
      #if(NOT BUILD_SHARED_LIBS)
          #--------------------------------------------------------------------
          # Append library directories to project list
          set(uniqueNewLibDirs ${${u_package}_LIBRARY_DIR})
          remove_from_list(uniqueNewLibDirs "${${u_package}_LIBRARY_DIR}" ${_allLibsDir})
          set(${u_package}_LIBRARY_DIR ${uniqueNewLibDirs})
          list(APPEND _allLibsDir ${${u_package}_LIBRARY_DIR})
          
          #--------------------------------------------------------------------
          # Append includes to project list
          set(uniqueNewLibs ${${u_package}_LIBRARIES})
          remove_library_list(uniqueNewLibs "${${u_package}_LIBRARIES}" ${_allLibs})
          set(${u_package}_LIBRARIES ${uniqueNewLibs})
          list (APPEND _allLibs ${${u_package}_LIBRARIES})
          
          #--------------------------------------------------------------------
          # Append definitions to project list
          set(uniqueNewDefs ${${u_package}_DEFINITIONS})
          remove_from_list(uniqueNewDefs "${${u_package}_DEFINITIONS}" ${_allDefinitions})
          set(${u_package}_DEFINITIONS ${uniqueNewDefs})
          list (APPEND _allDefinitions ${${u_package}_DEFINITIONS})

          #--------------------------------------------------------------------
          # Append link flags to project list
          set(uniqueNewLinkFlags ${${u_package}_LINK_FLAGS})
          remove_from_list(uniqueNewLinkFlags "${${u_package}_LINK_FLAGS}" ${_allLinkFlags})
          set(${u_package}_LINK_FLAGS ${uniqueNewLinkFlags})
          if(NOT "${${u_package}_LINK_FLAGS}" STREQUAL "")
              list (APPEND _allLinkFlags "\"${${u_package}_LINK_FLAGS}\"")
          endif()
      #endif()

      #--------------------------------------------------------------------
      # Set includes and append to list
      include_directories(${${u_package}_INCLUDE_DIR})
      list(APPEND _allIncludeDirs ${${u_package}_INCLUDE_DIR})
      include_directories(${${u_package}_INCLUDE_DIRS})
      list(APPEND _allIncludeDirs ${${u_package}_INCLUDE_DIRS})

      #--------------------------------------------------------------------
      # Set directory links
      link_directories(${${u_package}_LIBRARY_DIR})

      #--------------------------------------------------------------------
      # Set directory links
      add_definitions(${${u_package}_DEFINITIONS})
      
      #--------------------------------------------------------------------
      # Add dependency projects
      if(BUILD_${u_package})
          if(NOT DEFINED ${u_package}_PROJECT)
            set(${u_package}_PROJECT ${package})
          endif()
          add_dependencies(${_projectName} ${${u_package}_PROJECT})
      endif(BUILD_${u_package})
      
      #--------------------------------------------------------------------
      # Link library
       target_link_libraries(${_projectName} ${${u_package}_LIBRARIES})
      
      #--------------------------------------------------------------------
      # Add library to pack
      # if(IVW_PACKAGE_PROJECT)  
          # if(BUILD_SHARED_LIBS)
                # if(WIN32 AND CMAKE_BUILD_TYPE MATCHES DEBUG)
                    # set(BINARY_FILE ${${u_package}_LIBRARY_DIR}/${${u_package}_PROJECT}${CMAKE_DEBUG_POSTFIX}.${CMAKE_SHARED_LIBRARY_PREFIX})
                # else()
                    # set(BINARY_FILE ${${u_package}_LIBRARY_DIR}/${${u_package}_PROJECT}.${CMAKE_SHARED_LIBRARY_PREFIX})
                # endif()
               # #--------------------------------------------------------------------
               # # Add to package
               # install(FILES ${BINARY_FILE}
                         # RUNTIME
                         # DESTINATION bin)
           # endif()
       # endif()

      #--------------------------------------------------------------------
      # Link flags
      if(NOT "${${u_package}_LINK_FLAGS}" STREQUAL "")
      	set_target_properties(${_projectName} PROPERTIES LINK_FLAGS "${${u_package}_LINK_FLAGS}")
      endif()
      
      #--------------------------------------------------------------------
      # Qt5
       if(DESIRED_QT_VERSION MATCHES 5)
           if(${package} STREQUAL "InviwoOpenGLQtModule")
                qt5_use_modules(${_projectName} OpenGL)
           endif()
       endif()
      
    endforeach()
endmacro()
