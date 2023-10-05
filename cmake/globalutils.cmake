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

# This file contains a collection of CMake utility scripts

# first_case_upper(retval string)
# Make the first letter uppercase and the rest lower
function(first_case_upper retval value)
    string(TOLOWER ${value} lowercase)
    string(SUBSTRING ${lowercase} 0 1 first_letter)
    string(TOUPPER ${first_letter} first_letter)
    string(REGEX REPLACE "^.(.*)" "${first_letter}\\1" result "${lowercase}")
    set(${retval} ${result} PARENT_SCOPE)
endfunction()

function(lowercase retval)
    set(the_list "")
    foreach(item IN LISTS ARGN)
        string(TOLOWER ${item} lowercase)
        list(APPEND the_list ${lowercase})
    endforeach()
    set(${retval} ${the_list} PARENT_SCOPE)
endfunction()

# ivw_join(sep glue output)
# Joins list by replacing the separator with glue
function(ivw_join sep glue output)
    string (REGEX REPLACE "([^\\]|^)${sep}" "\\1${glue}" _TMP_STR "${ARGN}")
    # duplicate all backslashes
    string (REGEX REPLACE "[\\](.)" "\\1" _TMP_STR "${_TMP_STR}")
    set(${output} "${_TMP_STR}" PARENT_SCOPE)
endfunction()

# ivw_prepend(output str)
# ivw_prepends str to each element of the input
function(ivw_prepend var prefix)
   set(listVar "")
   foreach(f ${ARGN})
      list(APPEND listVar "${prefix}${f}")
   endforeach(f)
   set(${var} "${listVar}" PARENT_SCOPE)
endfunction(ivw_prepend)

# ivw_pad_right(output output str padchar length)
# ivw_pad_right add padding to the right of str
function(ivw_pad_right output str padchar length)
  string(LENGTH "${str}" _strlen)
  math(EXPR _strlen "${length} - ${_strlen}")
  if(_strlen GREATER 0)
    string(REPEAT ${padchar} ${_strlen} _pad)
    string(APPEND str ${_pad})
  endif()
  set(${output} "${str}" PARENT_SCOPE)
endfunction()

# Creates VS folder structure
function(ivw_folder target folder_name)
    set_target_properties(${target} PROPERTIES FOLDER ${folder_name})
endfunction()

# encodeLineBreaks(output strings)
# encodes the contents of the string given as last argument and saves the 
# result in output.
# Linebreaks ('\n'), semicolon (';'), and quotes ('"') are replaced for better handling 
# within CMAKE with __LINEBREAK__ and __SEMICOLON__, respectively.
function(encodeLineBreaks output)
    # replace linebreaks
    string(REPLACE "\n" "__LINEBREAK__" _tmp_str "${ARGN}")
    # replace semicolon, as it is interpreted as a list separator by CMAKE
    string(REPLACE ";" "__SEMICOLON__" _tmp_str "${_tmp_str}")
    # replace quotes as well
    string(REPLACE "\"" "__QUOTE__" _tmp_str "${_tmp_str}")
    set(${output} "${_tmp_str}" PARENT_SCOPE)
endfunction()

# decodeLineBreaks(output strings)
# reverse the encoding done by encodeLineBreaks(), i.e. __LINEBREAK__ and 
# __SEMICOLON__ are reverted to '\n' and ';', respectively.
function(decodeLineBreaks output)
    # revert linebreaks
    string(REPLACE "__LINEBREAK__" "\n" _tmp_str "${ARGN}")
    # revert semicolon
    string(REPLACE "__SEMICOLON__" ";" _tmp_str "${_tmp_str}")
    # revert quotes
    string(REPLACE "__QUOTE__" "\"" _tmp_str "${_tmp_str}")
    set(${output} "${_tmp_str}" PARENT_SCOPE)
endfunction()

# list_all_variables()
# prints all variables and values.
macro(list_all_variables)
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
endmacro()

# list_subdirectories(ret curdir return_relative)
# List subdirectories, excluding dirs starting with "."
function(list_subdirectories retval curdir return_relative)
    file(GLOB sub-dir RELATIVE ${curdir} ${curdir}/[^.]*)
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
    set(${retval} ${list_of_dirs} PARENT_SCOPE)
endfunction()

# remove_duplicates(retval item item ...)
# Clean duplicates from list subdirectories
function(remove_duplicates retval)
    set(list_of_dirs ${ARGN})
    if(list_of_dirs)
        list(REMOVE_DUPLICATES list_of_dirs)
    endif()
    set(${retval} ${list_of_dirs} PARENT_SCOPE)
endfunction()

# ivw_remove_from_list(retval thelist toRemove0 toRemove1 ...)
# Remove entries in one list from another list
function(ivw_remove_from_list retval thelist)
    set(new_items ${${thelist}})
    set(old_items ${ARGN})
    if(old_items AND new_items)
        foreach(item ${old_items})
            list(REMOVE_ITEM new_items ${item})
        endforeach()
    endif()
    set(${retval} ${new_items} PARENT_SCOPE)
endfunction()

# list_intersection(retval list_a list_b)
# return items that are in both lists
function(list_intersection retval list_a list_b)
    set(intersection "")
    list(REMOVE_DUPLICATES list_a)
    list(REMOVE_DUPLICATES list_b)
    
    foreach(item ${list_a})
        list(FIND list_b ${item} index)
        if(NOT index EQUAL -1)
            list(APPEND intersection ${item})
        endif()
    endforeach()
    set(${retval} ${intersection} PARENT_SCOPE)
endfunction()

# list_to_stringvector(retval item1 item2 ...) -> {"item1", "item2", ...}
# builds a string vector 
function(list_to_stringvector retval)
    set (items ${ARGN})
    list(LENGTH items len)
    if (${len} GREATER 0)
        ivw_join(";" "\", \"" res ${items})
        set(${retval} "{\"${res}\"}" PARENT_SCOPE)
    else()
        set(${retval} "{}" PARENT_SCOPE)
    endif()
endfunction()

function(list_to_longstringvector retval) # same but with linebreaks
    set (items ${ARGN})
    list(LENGTH items len)
    if (${len} GREATER 0)
        ivw_join(";" "\",\n    \"" res ${items})
        set(${retval} "{\n    \"${res}\"\n}" PARENT_SCOPE)
    else()
        set(${retval} "{}" PARENT_SCOPE)
    endif()
endfunction()

# mark_as_internal(var)
# hides the given variable in all CMake UIs
macro ( mark_as_internal _var )
  set ( ${_var} ${${_var}} CACHE INTERNAL "hide this!" FORCE )
endmacro( mark_as_internal _var )

# ivw_add_module_option_to_cache(mod ON/OFF [FORCE])
# add/update a module to the CMake cache
function(ivw_add_module_option_to_cache mod onoff)
    set(options "FORCE;ON;OFF")
    set(oneValueArgs "")
    set(multiValueArgs "")
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(desc "Build ${${mod}_name} Module\nPath: ${${mod}_path}")

    if(${mod}_version)
        set(desc "${desc}\nVersion: ${${mod}_version}")
    endif()
    if(${mod}_dependencies)
        ivw_join(";" ", " res ${${mod}_dependencies})
        set(desc "${desc}\nDependencies: ${res}")
    endif()    
    if(${mod}_description)
        decodeLineBreaks(decodedDesc ${${mod}_description})
        set(desc "${desc}\n\n${decodedDesc}")
    endif()

    if(ARG_FORCE)
        set(${${mod}_opt} ${onoff} CACHE BOOL "${desc}" FORCE)
    elseif(NOT DEFINED ${${mod}_opt})
        option(${${mod}_opt} "${desc}" ${onoff})
    else()
        # need to do this to update the docstring
        set(${${mod}_opt} ${${${mod}_opt}} CACHE BOOL "${desc}" FORCE)
    endif()
endfunction()

#--------------------------------------------------------------------
# Name conventions:
# opengl                 : dir                : Name of module folder, should be lowercase with no spaces (opengl)
# OpenGl                 : class              : The c++ class name / the module project name
# OPENGL                 : macro_name         : C Macro name, to uppercase, "-" -> "_"
# INVIWOOPENGLMODULE     : mod_dep            : Internal name for module all uppercase
# REG_INVIWOOPENGLMODULE : reg                : Registration macro
# IVW_MODULE_OPENGL      : mod_prefix         : Name of cmake option for module
# InviwoOpenGLModule     : mod_name           : The name of a module same as mod_dep, but not uppercase
# inviwo-module-opengl   : module_target_name : The name of the target for a module
# 
# Name conversion functions:
# ivw_to_macro_name            OpenGL-test        -> OPENGL_TEST
# ivw_dir_to_mod_dep           opengl             -> INVIWOOPENGLMODULE
# ivw_mod_dep_to_dir           INVIWOOPENGLMODULE -> opengl
# ivw_mod_dep_to_mod_name      INVIWOOPENGLMODULE -> InviwoOpenGLModule
# ivw_dir_to_mod_prefix        opengl             -> IVW_MODULE_OPENGL
# ivw_mod_prefix_to_dir        IVW_MODULE_OPENGL  -> opengl
# ivw_mod_name_to_dir          InviwoOpenGLModule -> opengl
# ivw_mod_name_to_target_name  InviwoOpenGLModule -> inviwo-module-opengl
# ivw_mod_name_to_class        InviwoOpenGLModule -> OpenGL
# ivw_mod_name_to_name         InviwoOpenGLModule -> OpenGL (Will return any input as is if it does not match)
# ivw_mod_name_to_mod_dep      InviwoOpenGLModule -> INVIWOOPENGLMODULE
# ivw_mod_name_to_reg          InviwoOpenGLModule -> REG_INVIWOOPENGLMODULE
# ivw_to_mod_name              OpenGL             -> InviwoOpenGLModule
# ivw_dir_to_module_taget_name opengl             -> inviwo-module-opengl
# ivw_mod_name_to_alias        InviwoOpenGLModule -> inviwo::module::opengl
#--------------------------------------------------------------------

# ivw_to_macro_name(retval item1 item2 ...)
# Convert a name to a macro name, i.e. OpenGL-test -> OPENGL_TEST
function(ivw_to_macro_name retval)
    set(the_list "")
    foreach(item IN LISTS ARGN)
        string(TOUPPER ${item} u_item)
        string(REGEX REPLACE "-" "_" new_item ${u_item})
        list(APPEND the_list "${new_item}")
    endforeach()
    set(${retval} ${the_list} PARENT_SCOPE)
endfunction()

# ivw_dir_to_mod_dep(retval item1 item2 ...)
# Convert directory name tp module dep, i.e. opengl -> INVIWOOPENGLMODULE
function(ivw_dir_to_mod_dep retval)
    set(the_list "")
    foreach(item IN LISTS ARGN)
        string(TOUPPER ${item} u_item)
        list(APPEND the_list "INVIWO${u_item}MODULE")
    endforeach()
    set(${retval} ${the_list} PARENT_SCOPE)
endfunction()

# ivw_dir_to_mod_prefix(retval item1 item2 ...)
# Convert dir name to  module prefix, i.e. opengl -> IVW_MODULE_OPENGL
function(ivw_dir_to_mod_prefix retval)
    set(the_list "")
    foreach(item IN LISTS ARGN)
        string(TOUPPER ${item} u_item)
        list(APPEND the_list IVW_MODULE_${u_item})
    endforeach()
    set(${retval} ${the_list} PARENT_SCOPE)
endfunction()

# ivw_mod_dep_to_dir(relval item1 item2 ...)
# Convert module dep to directory name, i.e. INVIWOOPENGLMODULE -> opengl
function(ivw_mod_dep_to_dir retval)
    set(the_list "")
    foreach(item IN LISTS ARGN)
        string(REGEX MATCH "^INVIWO(.*)MODULE$" found_item ${item})
        if(CMAKE_MATCH_1)
            string(TOLOWER ${CMAKE_MATCH_1} l_new_item)
            list(APPEND the_list ${l_new_item})
        endif()
    endforeach()
    set(${retval} ${the_list} PARENT_SCOPE)
endfunction()

# ivw_mod_prefix_to_dir(relval item1 item2 ...)
# Convert module prefix to directory name, i.e. IVW_MODULE_OPENGL -> opengl
function(ivw_mod_prefix_to_dir retval)
    set(the_list "")
    foreach(item IN LISTS ARGN)
        string(REGEX MATCH "(^IVW_MODULE_$)" found_item ${item})
        if(found_item)
            string(REGEX REPLACE "(^IVW_MODULE_$)" "" new_item ${item})
            string(TOLOWER ${new_item} l_new_item)
            list(APPEND the_list ${l_new_item})
        endif()
    endforeach()
    set(${retval} ${the_list} PARENT_SCOPE)
endfunction()

# ivw_mod_name_to_dir(retval item1 item2 ...)
# Convert module name to directory name, i.e. InviwoOpenGLModule -> opengl
function(ivw_mod_name_to_dir retval)
    set(the_list "")
    foreach(item IN LISTS ARGN)
        string(REGEX MATCH "(^Inviwo.*.Module$)" found_item ${item})
        if(found_item)
            string(REGEX REPLACE "(^Inviwo)|(Module$)" "" new_item ${item})
            string(TOLOWER ${new_item} l_new_item)
            list(APPEND the_list ${l_new_item})
        else()
            message(FATAL_ERROR "Error argument format error: ${item}, should be in the form Inviwo<Name>Module")
        endif()
    endforeach()
    set(${retval} ${the_list} PARENT_SCOPE)
endfunction()

# ivw_mod_name_to_class(retval item1 item2 ...)
# Convert module name to directory name, i.e. InviwoOpenGLModule -> OpenGL
function(ivw_mod_name_to_class retval)
    set(the_list "")
    foreach(item IN LISTS ARGN)
        string(REGEX MATCH "(^Inviwo.*.Module$)" found_item ${item})
        if(found_item)
            string(REGEX REPLACE "(^Inviwo)|(Module$)" "" new_item ${item})
            list(APPEND the_list ${new_item})
        else()
            message(FATAL_ERROR "Error argument format error: ${item}, should be in the form Inviwo<Name>Module")
        endif()
    endforeach()
    set(${retval} ${the_list} PARENT_SCOPE)
endfunction()

# ivw_mod_name_to_name(retval item1 item2 ...)
# Convert module name to directory name, i.e. InviwoOpenGLModule -> OpenGL
# Will return any input as is if it does not match
function(ivw_mod_name_to_name retval)
    set(the_list "")
    foreach(item IN LISTS ARGN)
        string(REGEX MATCH "(^Inviwo.*.Module$)" found_item ${item})
        if(found_item)
            string(REGEX REPLACE "(^Inviwo)|(Module$)" "" new_item ${item})
            list(APPEND the_list ${new_item})
        else()
            list(APPEND the_list ${item})
        endif()
    endforeach()
    set(${retval} ${the_list} PARENT_SCOPE)
endfunction()

# ivw_mod_name_to_alias(retval item1 item2 ...)
# Convert module name to alias name, i.e. InviwoOpenGLModule -> inviwo::module::alias
# using module data
function(ivw_mod_name_to_alias retval)
    set(the_list "")
    foreach(item IN LISTS ARGN)
        ivw_mod_name_to_mod_dep(mod ${item})
        list(APPEND the_list ${${mod}_alias})
    endforeach()
    set(${retval} ${the_list} PARENT_SCOPE)
endfunction()

# ivw_mod_dep_to_mod_name(retval item1 item2 ...)
# Convert module name to alias name, i.e. INVIWOOPENGLMODULE -> InviwoOpenGLModule
# using module data
function(ivw_mod_dep_to_mod_name retval)
    set(the_list "")
    foreach(mod in LISTS ARGN)
        list(APPEND the_list ${${mod}_modName})
    endforeach()
    set(${retval} ${the_list} PARENT_SCOPE)
endfunction()

# ivw_mod_name_to_target_name(retval item1 item2 ...)
# Convert module name to target name, i.e. InviwoOpenGLModule -> inviwo-opengl-module
function(ivw_mod_name_to_target_name retval)
    set(dirs "")
    ivw_mod_name_to_dir(dirs ${ARGN})
    set(targets "")
    ivw_dir_to_module_taget_name(targets ${dirs})
    set(${retval} ${targets} PARENT_SCOPE)
endfunction()

# ivw_to_mod_name(retval item1 item2 ...)
# Convert module name to directory name, i.e. OpenGL -> InviwoOpenGLModule
function(ivw_to_mod_name retval)
    set(the_list "")
    foreach(item IN LISTS ARGN)
        list(APPEND the_list Inviwo${item}Module)
    endforeach()
    set(${retval} ${the_list} PARENT_SCOPE)
endfunction()

# ivw_dir_to_module_taget_name(retval item1 item2 ...)
# Convert module name to directory name, i.e. opengl -> inviwo-module-opengl
function(ivw_dir_to_module_taget_name retval)
    set(the_list "")
    foreach(item IN LISTS ARGN)
        list(APPEND the_list inviwo-module-${item})
    endforeach()
    set(${retval} ${the_list} PARENT_SCOPE)
endfunction()

# ivw_mod_name_to_mod_dep(retval item1 item2 ...)
# Convert module name to module dep, i.e. InviwoOpenGLModule -> INVIWOOPENGLMODULE
function(ivw_mod_name_to_mod_dep retval)
    set(the_list "")
    foreach(item IN LISTS ARGN)
        string(TOUPPER ${item} uitem)
        list(APPEND the_list ${uitem})
    endforeach()
    set(${retval} ${the_list} PARENT_SCOPE)
endfunction()

# ivw_mod_name_to_reg(retval item1 item2 ...)
# Convert module name to module dep, i.e. InviwoOpenGLModule -> REG_INVIWOOPENGLMODULE
function(ivw_mod_name_to_reg retval)
    set(the_list "")
    foreach(item IN LISTS ARGN)
        string(TOUPPER ${item} uitem)
        list(APPEND the_list REG_${uitem})
    endforeach()
    set(${retval} ${the_list} PARENT_SCOPE)
endfunction()

# Look for <package>_<vars> and <PACKAGE>_<vars> to figure out whether to use
# incoming casing or upper case name. Upper case is default, but not all use that.
function(ivw_find_package_name name retval)
    set(input 0)
    set(upper 0)

    string(TOUPPER ${name} u_name)

    foreach(prefix FOUND LIBRARY LIBRARIES LIBRARY_DIR LIBRARY_DIRS 
        DEFINITIONS INCLUDE_DIR INCLUDE_DIRS LINK_FLAGS) 
    if(DEFINED ${name}_${prefix})
        MATH(EXPR input "${input}+1")
    endif()
    if(DEFINED ${u_name}_${prefix})
        MATH(EXPR upper "${upper}+1")
    endif()
    endforeach()

    if(${input} GREATER ${upper})
        set(${retval} ${name} PARENT_SCOPE)
    else()
        set(${retval} ${u_name} PARENT_SCOPE)
    endif()
endfunction()

# pritty print a list .
# ivw_print_list(list) -> "list  = list1, list2, list3"
function(ivw_print_list list_var)
    string(REPLACE ";" ", " res "${${list_var}}") 
    message(STATUS "${list_var} = ${res}")
endfunction()

# create a reverse copy of a list
# ivw_reverse_copy(retval list) 
function(ivw_reverse_list_copy list_var revlist)
    set(alist "")
    foreach(i ${${list_var}})
        list(APPEND alist ${i})
    endforeach()
    list(REVERSE alist)
    set(${revlist} ${alist} PARENT_SCOPE)
endfunction()

# Assign one of two values to a variable as a result of a condition
# ivw_set_if(COND <bool value> TRUEVAL <if true> FALSEVAL <if false> RETVAL <output var>)
function(ivw_set_if)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "RETVAL;TRUEVAL;FALSEVAL" "COND")
    if(${ARG_COND})
        set(${ARG_RETVAL} ${ARG_TRUEVAL} PARENT_SCOPE)
    else()
        set(${ARG_RETVAL} ${ARG_FALSEVAL} PARENT_SCOPE)
    endif()
endfunction()

# helper function for topological sort
function(ivw_private_visit_node node sorted_var marked_var tempmarked_var node_list_var node_edge_var count)
    MATH(EXPR count "${count}+1")
    if(${count} GREATER 30)
        message(ERROR "Stoppig to deep recursion")
        return()
    endif()

    set(sorted ${${sorted_var}})
    set(marked ${${marked_var}})
    set(tempmarked ${${tempmarked_var}})
    set(node_list ${${node_list_var}})

    list(FIND tempmarked ${node} tempfound)
    if(NOT ${tempfound} EQUAL -1) # Error not a dag
        message(ERROR "Dependency graph not a DAG. Cant resove for node \"${node}\"")
    endif()

    list(FIND marked ${node} markedfound)
    if(${markedfound} EQUAL -1)  
        # mark node temporarily
        list(APPEND tempmarked ${node})

        # visit all dependencies
        foreach(dep ${${node}${node_edge_var}}) 
            ivw_private_visit_node(${dep} sorted marked tempmarked node_list ${node_edge_var} ${count})
        endforeach()

        # mark node permanently
        list(APPEND marked ${node})
        # unmark node temporarily
        list(REMOVE_ITEM tempmarked ${node})

        # add node to head of sorted
        list(INSERT sorted 0 ${node})
     endif()

    SET(${sorted_var} ${sorted} PARENT_SCOPE)
    SET(${marked_var} ${marked} PARENT_SCOPE)
    SET(${tempmarked_var} ${tempmarked} PARENT_SCOPE)
endfunction()

# A depth first topologocal sort
# https://en.wikipedia.org/wiki/Topological_sorting#Depth-first_search
function(ivw_topological_sort node_list_var node_edge_var sorted_var)
    set(sorted  "")
    set(marked "")
    set(tempmarked "")

    foreach(node ${${node_list_var}})
        list(FIND marked ${node} found)
        if(${found} EQUAL -1)
            ivw_private_visit_node(${node} sorted marked tempmarked ${node_list_var} ${node_edge_var} 0)
        endif()
    endforeach()
    list(REVERSE sorted)
    set(${sorted_var} ${sorted} PARENT_SCOPE)
endfunction()

# Get the module name from a CMakeLists.txt
function(ivw_private_get_ivw_module_name path name_var version_var)
    file(READ ${path} contents)
    string(REGEX MATCH "ivw_module\\([ \t\n\r]*([A-Za-z0-9_-]+)[ \t\n\r]*([ \t\n\r]+VERSION[ \t\n\r]+([0-9]+)\\.([0-9]+)\\.([0-9]+)[ \t\n\r]*)?\\)" found_item ${contents})
    if(found_item AND CMAKE_MATCH_1)
        set(${name_var} ${CMAKE_MATCH_1} PARENT_SCOPE)
        if(CMAKE_MATCH_3 AND CMAKE_MATCH_4 AND CMAKE_MATCH_5)
            set(${version_var} "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}" PARENT_SCOPE)
        else()
            set(${version_var} "1.0.0" PARENT_SCOPE)
        endif()
        return()
    endif()
     set(${name_var} NOTFOUND PARENT_SCOPE)
     set(${version_var} NOTFOUND PARENT_SCOPE)
endfunction()

# Get the module include path
function(ivw_private_get_ivw_module_include_path path includePrefix includePath orgName)
    get_filename_component(name ${path} NAME)
    if(EXISTS "${path}/include/${name}") 
        set(${includePath} "${path}/include/${name}" PARENT_SCOPE)
        set(${includePrefix} "${name}" PARENT_SCOPE)
        set(${orgName} "" PARENT_SCOPE)
        return()
    endif()
    
    if(EXISTS "${path}/include/")
        file(GLOB subdirs RELATIVE "${path}/include/" "${path}/include/[^.]*")
        foreach(item IN LISTS subdirs)
            if(EXISTS "${path}/include/${item}/${name}")
                set(${includePath} "${path}/include/${item}/${name}" PARENT_SCOPE)
                set(${includePrefix} "${item}/${name}" PARENT_SCOPE)
                set(${orgName} "${item}" PARENT_SCOPE)
                return()
            endif()
        endforeach()
    endif()
    
    set(${includePath} "${path}" PARENT_SCOPE)
    set(${includePrefix} "modules/${name}" PARENT_SCOPE)
    set(${orgName} "modules" PARENT_SCOPE)
    
endfunction()

# Verify that a given path and dir name is in fact a inviwo module
# This is done by chekcing that there exists a CMakeLists file
# and that it declares a inviwo module with the same name as dir.
function(ivw_private_is_valid_module_dir)
    set(options )
    set(oneValueArgs PATH DIR VALID_VAR NAME_VAR VERSION_VAR)
    set(multiValueArgs)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    foreach(item IN LISTS oneValueArgs)
        if(NOT ARG_${item})
            message(FATAL_ERROR "ivw_private_is_valid_module_dir: ${item} not set")
        endif()
    endforeach()

    if(IS_DIRECTORY ${ARG_PATH}/${ARG_DIR})
        string(TOLOWER ${ARG_DIR} test)
        string(REPLACE " " "" ${test} test)
        if(${ARG_DIR} STREQUAL ${test})
            if(EXISTS ${ARG_PATH}/${ARG_DIR}/CMakeLists.txt)
                ivw_private_get_ivw_module_name(${ARG_PATH}/${ARG_DIR}/CMakeLists.txt name version)
                string(TOLOWER ${name} l_name)
                if(${ARG_DIR} STREQUAL ${l_name})
                    set(${ARG_VALID_VAR} TRUE PARENT_SCOPE)
                    set(${ARG_NAME_VAR} ${name} PARENT_SCOPE)
                    set(${ARG_VERSION_VAR} ${version} PARENT_SCOPE)
                    return()
                else()
                    message("Found invalid module \"${ARG_DIR}\" at \"${ARG_PATH}\". "
                        "ivw_module called with \"${name}\" which is different from the directory \"${ARG_DIR}\""
                        "They should be the same except for casing.")
                endif()
            else()
                message("Found invalid module \"${ARG_DIR}\" at \"${ARG_PATH}\". "
                    "CMakeLists.txt is missing")
            endif()
        else()
            message("Found invalid module dir \"${ARG_DIR}\" at \"${ARG_PATH}\". "
                "Dir names should be all lowercase and without spaces")
        endif()
    endif()
    set(${ARG_VALID_VAR} FALSE PARENT_SCOPE)
    set(${ARG_NAME_VAR} NOTFOUND PARENT_SCOPE)
    set(${ARG_VERSION_VAR} NOTFOUND PARENT_SCOPE)
endfunction()

# Query if a lib is compiled with 32 or 64 bits, will return 0 if it 
# could not find out. 
function(ivw_library_bits lib retval)
    if(WIN32)
        get_filename_component(vcpath ${CMAKE_CXX_COMPILER} DIRECTORY)
        execute_process(COMMAND CMD /c dumpbin.exe ${lib} /headers | findstr machine 
                        WORKING_DIRECTORY ${vcpath} 
                        OUTPUT_VARIABLE result)
 
        string(REGEX MATCH "(x64)" found_64bit ${result})
        if(CMAKE_MATCH_1)
            set(${retval} 64 PARENT_SCOPE)
            return()
        endif()
        string(REGEX MATCH "(x86)" found_32bit ${result})
        if(CMAKE_MATCH_1)
            set(${retval} 32 PARENT_SCOPE)
            return()
        endif()
    elseif(APPLE)
        execute_process(COMMAND file -L -b ${lib} OUTPUT_VARIABLE result)
        string(REGEX MATCH "(x86_64)" found_64bit ${result})
        if(CMAKE_MATCH_1)
            set(${retval} 64 PARENT_SCOPE)
            return()
        endif()
        string(REGEX MATCH "(i386)" found_32bit ${result})
        if(CMAKE_MATCH_1)
            set(${retval} 32 PARENT_SCOPE)
            return()
        endif()
    else()
        execute_process(COMMAND file -L -b ${lib} OUTPUT_VARIABLE result)
        string(REGEX MATCH "(x86-64)" found_64bit ${result})
        if(CMAKE_MATCH_1)
            set(${retval} 64 PARENT_SCOPE)
            return()
        endif()
        string(REGEX MATCH "(80386)" found_32bit ${result})
        if(CMAKE_MATCH_1)
            set(${retval} 32 PARENT_SCOPE)
            return()
        endif()
    endif()

    set(${retval} 0 PARENT_SCOPE)
endfunction()

# From a list of paths find the unique path segments
function(ivw_find_unique_path_segements retval paths)
    list(LENGTH paths npaths)

    # Remove non-unique start of path
    set(ind 0)
    set(names ${paths})
    set(ret ${paths})
    list(LENGTH names n_names)
    while(n_names EQUAL npaths)
        set(ret ${names})
        set(names "")
        foreach(module ${paths})
            set(path "")
            set(i 0)
            string(REPLACE "/" ";" module_list ${module})
            foreach(dir ${module_list})
                if( i GREATER ind OR i EQUAL ind)
                    list(APPEND path ${dir})
                endif()
                MATH(EXPR i "${i}+1")
            endforeach()
            string(REPLACE ";" "/" path_joined "${path}")
            list(APPEND names ${path_joined})
        endforeach()
        list(REMOVE_DUPLICATES names)
        list(LENGTH names n_names)
        MATH(EXPR ind "${ind}+1")
    endwhile()
    
    # Remove non-unique end of path
    set(ind 0)
    set(new_module_bases ${ret})
    set(names ${ret})
    list(LENGTH names n_names)
    while(n_names EQUAL npaths)
        set(ret ${names})
        set(names "")
        foreach(module ${new_module_bases})
            set(path "")
            set(i 0)
            string(REPLACE "/" ";" module_list ${module})
            list(REVERSE module_list)
            foreach(dir ${module_list})
                if( i GREATER ind OR i EQUAL ind)
                    list(APPEND path ${dir})
                endif()
                MATH(EXPR i "${i}+1")
            endforeach()
            list(REVERSE path)
            string(REPLACE ";" "/" path_joined "${path}")
            list(APPEND names ${path_joined})
        endforeach()
        list(REMOVE_DUPLICATES names)
        list(LENGTH names n_names)
        MATH(EXPR ind "${ind}+1")
    endwhile()
    set(${retval} ${ret} PARENT_SCOPE)
endfunction()

# A function to try to retrive the git hash of the last commit in a 
# directory
function(ivw_git_get_hash dir retval)
    find_package(Git QUIET)
    if(GIT_FOUND)
        execute_process(COMMAND "${GIT_EXECUTABLE}" describe --match=NeVeRmAtCh --always --abbrev=20 --dirty
            WORKING_DIRECTORY ${dir}
            RESULT_VARIABLE result
            OUTPUT_VARIABLE version
            ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
        if(${result} EQUAL 0)
            set(${retval} ${version} PARENT_SCOPE)
            return()
        endif()
    endif()
    set(${retval} "????????" PARENT_SCOPE)
endfunction()

# A helper funtion to generate a header file with inviwo build 
# information, like the build date and the commit hash
# ivw_generate_build_info(<template> <outputfile> <module dir1> <module dir2> ...
function(ivw_generate_build_info source_template ini_template buildinfo_sourcefile buildinfo_inifile)
    ivw_find_unique_path_segements(unique_names "${ARGN}")
    set(index 0)
    set(hashes_list "")
    foreach(dir ${ARGN})
        list(GET unique_names ${index} name)
        ivw_git_get_hash(${dir} hash)
        list(APPEND hashes_list "{\"${name}\", \"${hash}\"}")
        MATH(EXPR index "${index}+1")
    endforeach()
    string(REPLACE ";" ",\n            " hashes "${hashes_list}")
    set(HASHES "{\n            ${hashes}\n        }")
    set(NHASHES "${index}")

    set(index 0)
    set(hashes_list "")
    foreach(dir ${ARGN})
        list(GET unique_names ${index} name)
        ivw_git_get_hash(${dir} hash)
        list(APPEND hashes_list "${name}=${hash}")
        MATH(EXPR index "${index}+1")
    endforeach()
    string(REPLACE ";" "\n" hashes "${hashes_list}")
    set(INIHASHES "${hashes}")


    string(TIMESTAMP TMPYEAR "%Y")
    string(REGEX REPLACE "0*([0-9]+)" "\\1" YEAR ${TMPYEAR})
    string(TIMESTAMP TMPMONTH "%m")
    string(REGEX REPLACE "0*([0-9]+)" "\\1" MONTH ${TMPMONTH})
    string(TIMESTAMP TMPDAY "%d")
    string(REGEX REPLACE "0*([0-9]+)" "\\1" DAY ${TMPDAY})
    string(TIMESTAMP TMPHOUR "%H")
    string(REGEX REPLACE "0*([0-9]+)" "\\1" HOUR ${TMPHOUR})
    string(TIMESTAMP TMPMINUTE "%M")
    string(REGEX REPLACE "0*([0-9]+)" "\\1" MINUTE ${TMPMINUTE})
    string(TIMESTAMP TMPSECOND "%S")
    string(REGEX REPLACE "0*([0-9]+)" "\\1" SECOND ${TMPSECOND})
    configure_file("${source_template}" "${buildinfo_sourcefile}" @ONLY)

    string(REPLACE "\"" "" ini_dest_path ${INI_DEST_PATH})
    configure_file("${ini_template}" "${ini_dest_path}${buildinfo_inifile}" @ONLY)
endfunction()

# Get target properties recursively by following all INTERFACE_LINK_LIBRARIES
function(ivw_get_target_property_recursive retval target property alsoInterfaceTargets)
    set(res "")
    get_target_property(target_type ${target} TYPE)
    if(NOT ${target_type} STREQUAL "INTERFACE_LIBRARY" OR ${alsoInterfaceTargets})
        get_target_property(propval ${target} ${property})
        if(propval)
            list(APPEND res ${propval})
        endif()

        get_target_property(interface_link_libs ${target} INTERFACE_LINK_LIBRARIES)
        foreach(t ${interface_link_libs})
            if(TARGET ${t})
                ivw_get_target_property_recursive(val ${t} ${property} ${alsoInterfaceTargets})
                list(APPEND res ${val})
            endif()
        endforeach(t)
    endif()
    list(REMOVE_DUPLICATES res)
    set(${retval} ${res} PARENT_SCOPE)
endfunction()

function(ivw_move_targets_in_dir_to_folder directory folder)
    get_property(targets DIRECTORY ${directory} PROPERTY BUILDSYSTEM_TARGETS)
    foreach(target IN LISTS targets)
        get_target_property(type ${target} TYPE)
        if(NOT ${type} STREQUAL INTERFACE_LIBRARY)
            set_target_properties(${target} PROPERTIES FOLDER ${folder})
        endif()
    endforeach()

    get_property(dirs DIRECTORY ${directory} PROPERTY SUBDIRECTORIES)
    foreach(dir IN LISTS dirs) 
        ivw_move_targets_in_dir_to_folder(${dir} ${folder})
    endforeach()
endfunction()

function(ivw_suppress_warnings_for_targets_in_dir directory)
    get_property(targets DIRECTORY ${directory} PROPERTY BUILDSYSTEM_TARGETS)
    foreach(target IN LISTS targets)
        get_target_property(type ${target} TYPE)
        if(NOT ${type} STREQUAL INTERFACE_LIBRARY)
            ivw_suppress_compiler_warnings(${target})
        endif()
    endforeach()

    get_property(dirs DIRECTORY ${directory} PROPERTY SUBDIRECTORIES)
    foreach(dir IN LISTS dirs) 
        ivw_suppress_warnings_for_targets_in_dir(${dir} ${folder})
    endforeach()
endfunction()

function(ivw_print_targets_in_dir_recursive directory)
    get_property(targets DIRECTORY ${directory} PROPERTY BUILDSYSTEM_TARGETS)
    message(STATUS "${directory}: ${targets}")
    
    get_property(dirs DIRECTORY ${directory} PROPERTY SUBDIRECTORIES)
    foreach(dir IN LISTS dirs) 
        ivw_print_targets_in_dir_recursive(${dir})
    endforeach()
endfunction()

function(ivw_get_targets_in_dir_recursive retval directory)
    set(res "")
    get_property(targets DIRECTORY ${directory} PROPERTY BUILDSYSTEM_TARGETS)
    list(APPEND res ${targets})
    
    get_property(dirs DIRECTORY ${directory} PROPERTY SUBDIRECTORIES)
    foreach(dir IN LISTS dirs) 
        ivw_get_targets_in_dir_recursive(targets ${dir})
        list(APPEND res ${targets})
    endforeach()
    set(${retval} ${res} PARENT_SCOPE)
endfunction()

function(ivw_copy_if retval)
    set(options EVAL NOT)
    set(oneValueArgs LIST PROJECTOR VALUE COMPARE)
    set(multiValueArgs "")
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ARG_LIST)
        message(FATAL_ERROR "ivw_copy_if: LIST not set")
    endif()
    if(ARG_NOT)
        set(negate NOT)
    else()
        set(negate "")
    endif()
    if(ARG_COMPARE)
        set(compare ${ARG_COMPARE})
    else()
        set(compare STREQUAL)
    endif()

    set(res "")
    
    if(ARG_PROJECTOR AND ARG_VALUE)
        if (ARG_EVAL)
            foreach(item IN LISTS ${ARG_LIST})
                if (${negate} "${${${item}${ARG_PROJECTOR}}}" ${compare} "${ARG_VALUE}")
                    list(APPEND res ${item})
                endif()
            endforeach()
        else()
            foreach(item IN LISTS ${ARG_LIST})
                if (${negate} "${${item}${ARG_PROJECTOR}}" ${compare} "${ARG_VALUE}")
                    list(APPEND res ${item})
                endif()
            endforeach()
        endif()
    elseif(ARG_PROJECTOR)
        if(ARG_EVAL)
            foreach(item IN LISTS ${ARG_LIST})
                if (${negate} "${${${item}${ARG_PROJECTOR}}}")
                    list(APPEND res ${item})
                endif()
            endforeach()
        else()
            foreach(item IN LISTS ${ARG_LIST})
                if (${negate} "${${item}${ARG_PROJECTOR}}")
                    list(APPEND res ${item})
                endif()
            endforeach()
        endif()
    elseif(ARG_VALUE)
        if(ARG_EVAL)
            foreach(item IN LISTS ${ARG_LIST})
                if (${negate} "${${item}}" ${compare} "${ARG_VALUE}")
                    list(APPEND res ${item})
                endif()
            endforeach()
        else()
            foreach(item IN LISTS ${ARG_LIST})
                if (${negate} "${item}" ${compare} "${ARG_VALUE}")
                    list(APPEND res ${item})
                endif()
            endforeach()
        endif()
    else()
        if(ARG_EVAL)
            foreach(item IN LISTS ${ARG_LIST})
                if (${negate} ${${item}})
                    list(APPEND res ${item})
                endif()
            endforeach()
        else()
            foreach(item IN LISTS ${ARG_LIST}) 
                if (${negate} ${item})
                    list(APPEND res ${item})
                endif()
            endforeach()
        endif()
    endif()
    set(${retval} ${res} PARENT_SCOPE)
endfunction()

# Uses QT's windeployqt.exe to copy necessary QT-dependencies (dlls etc) 
# for the given target to the build folder for development purposes. 
# Does nothing on platforms other than Windows. 
# Use ivw_default_install_targets (installutils.cmake) for deployment.
function(ivw_deploy_qt target)
    if(WIN32)
        get_target_property(target_type ${target} TYPE)
        # For dll-builds (ie BUILD_SHARED_LIBS == true) we need to run it for both .dll and .exe
        # For lib-builds (ie BUILD_SHARED_LIBS == false) we need to run it for only .exe (there are no .dll)
        if (BUILD_SHARED_LIBS OR (target_type STREQUAL "EXECUTABLE")) 
            find_program(WINDEPLOYQT_EXECUTABLE NAMES windeployqt HINTS ${QTDIR} ENV QTDIR PATH_SUFFIXES bin)

            get_filename_component(qt_bin_dir ${WINDEPLOYQT_EXECUTABLE} DIRECTORY  )

            # in case of environment variable QTDIR not set
            if(NOT EXISTS ${WINDEPLOYQT_EXECUTABLE})
                get_target_property(qmake_executable Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
                get_filename_component(qt_bin_dir "${qmake_executable}" DIRECTORY)
                find_program(WINDEPLOYQT_EXECUTABLE NAMES windeployqt HINTS ${qt_bin_dir} )
            endif()
            # Copy to build folder
            add_custom_command(
                TARGET ${target} POST_BUILD 
                COMMAND ${WINDEPLOYQT_EXECUTABLE} 
                    --no-compiler-runtime 
                    --no-translations
                    --no-system-d3d-compiler
                    --no-opengl-sw
                    --pdb
                    --verbose 1 
                    $<TARGET_FILE:${target}>
            )    
        endif()
    endif()
endfunction()

# Formats a string using python-like format strings.
# Format strings contain ?replacement fields? surrounded by curly braces {}.
# Anything that is not contained in braces is considered literal text, 
# which is copied unchanged to the output. If you need to include a brace 
# character in the literal text, it can be escaped by doubling: {{ and }}.
# 
# The grammar for a replacement field is as follows:
# replacement_field ::=  "{" [arg_id] [":" format_spec] "}"
# arg_id            ::=  integer
# integer           ::=  digit+
# digit             ::=  "0"..."9"
# format_spec       ::=  [[fill]align][width]
# fill              ::=  <a character other than '{' or '}'>
# align             ::=  "<" | ">" | "^"
# width             ::=  integer
# 
# The result can ether be returned in a variable given by RESULT
# or printed right away by passing one of the message types
# 
# Example:
#    ivw_format(RESULT str
#        FORMAT "some string '{0:>10}' more string '{1:_^11}' even move '{2:<10}'" 
#            " multiple format strings will be concatenated '{0:10}'" 
#        ARGUMENTS "Arg0" "Arg1" "Arg2")
# 
#    ivw_format(STATUS FORMAT "will be printed using message(STATUS) '{:*^10}'" ARGUMENTS "Arg0")
# 
function(ivw_format)
    set(options FATAL_ERROR SEND_ERROR WARNING AUTHOR_WARNING DEPRECATION NOTICE STATUS VERBOSE DEBUG TRACE)
    set(oneValueArgs RESULT)
    set(multiValueArgs FORMAT ARGUMENTS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ARG_FORMAT)
        message(FATAL_ERROR "ivw_format: FORMAT not set")
    endif()
    if(ARG_KEYWORDS_MISSING_VALUES) 
        message(FATAL_ERROR "ivw_format: Missing values for keywords ${ARG_KEYWORDS_MISSING_VALUES}")
    endif()
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "ivw_format: Unparsed arguments ${ARG_UNPARSED_ARGUMENTS}")
    endif()

    set(message_type "")
    foreach(item IN LISTS options)
        if(ARG_${item})
            list(APPEND message_type ${item})
        endif()
    endforeach()
    list(LENGTH message_type message_type_length)
    if(message_type_length GREATER 1)
        message(FATAL_ERROR "ivw_format: Only one message type can be set, got '${message_type}'")
    endif()

    list(LENGTH ARG_ARGUMENTS vars_length)

    set(result "")
    set(count 0)

    string(CONCAT format ${ARG_FORMAT})
    string(LENGTH "${format}" format_length)
    set(curr 0)
    while(curr LESS ${format_length})
        math(EXPR next "${curr} + 1")
        string(SUBSTRING "${format}" ${curr} 1 char)
        string(SUBSTRING "${format}" ${next} 1 nextChar)
        
        ## Handle exscaped { and }
        if(char STREQUAL "{" AND nextChar STREQUAL "{")
            string(APPEND result "{")
            math(EXPR curr "${curr} + 2")
        elseif(char STREQUAL "}" AND nextChar STREQUAL "}")
            string(APPEND result "}")
            math(EXPR curr "${curr} + 2")

        #  Handle format argument
        elseif(char STREQUAL "{")
            set(index "")
            set(fill " ")
            set(align "<")
            set(size "")

            # find argument index
            math(EXPR curr "${curr} + 1")
            string(SUBSTRING "${format}" ${curr} 1 char)
            while(curr LESS ${format_length} AND NOT char STREQUAL "}" AND NOT char STREQUAL ":")
                if(NOT char MATCHES [0-9])
                    message(FATAL_ERROR "ivw_format: Index must be a number, got '${char}'")
                endif()
                string(APPEND index ${char})
                math(EXPR curr "${curr} + 1")
                string(SUBSTRING "${format}" ${curr} 1 char)
            endwhile()
            if(index STREQUAL "")
                set(index ${count})
            endif()

            # find argument fill, alignment and size
            if(char STREQUAL ":")
                math(EXPR curr "${curr} + 1")
                math(EXPR next "${curr} + 1")
                string(SUBSTRING "${format}" ${curr} 1 char)
                string(SUBSTRING "${format}" ${next} 1 nextChar)
                if(nextChar MATCHES [<>^])
                    set(fill ${char})
                    set(align ${nextChar})
                    math(EXPR curr "${curr} + 2")
                elseif(char MATCHES [<>^])
                    set(align ${char})
                    math(EXPR curr "${curr} + 1")
                endif()

                string(SUBSTRING "${format}" ${curr} 1 char)
                while(curr LESS ${format_length} AND NOT char STREQUAL "}")
                    if(char MATCHES [0-9])
                        string(APPEND size ${char})
                    else()
                        message(FATAL_ERROR "ivw_format: Size must be a number, got '${char}'")
                    endif()

                    math(EXPR curr "${curr} + 1")
                    string(SUBSTRING "${format}" ${curr} 1 char)
                endwhile()
            endif()

            # get the indexed argument
            if(index GREATER_EQUAL vars_length)
                math(EXPR expected "${index} + 1")
                message(FATAL_ERROR "Missing variables in ivw_format, got ${vars_length} but expected at least ${expected}\nARGUMENTS: ${ARG_ARGUMENTS}")
            endif()
            list(GET ARG_ARGUMENTS ${index} var_name)

            # calculate padding
            string(LENGTH ${var_name} var_name_length)
            math(EXPR padding_length "${size} - ${var_name_length}")
            if(padding_length LESS 0)
                set(padding_length 0)
            endif()
            if(align STREQUAL "^")
                math(EXPR padd_right "${padding_length} / 2")
                math(EXPR padd_left "${padding_length} / 2 + ${padding_length} % 2")
            elseif(align STREQUAL "<")
                set(padd_right 0)
                set(padd_left ${padding_length})
            elseif(align STREQUAL ">")
                set(padd_right ${padding_length})
                set(padd_left 0)
            endif()

            # append padded argument to result
            string(REPEAT ${fill} ${padd_right} padding)
            string(APPEND result ${padding})
            string(APPEND result ${var_name})
            string(REPEAT ${fill} ${padd_left} padding)
            string(APPEND result ${padding})

            math(EXPR count "${count} + 1")
            math(EXPR curr "${curr} + 1")

        # pass normal char directly to result
        else()
            string(APPEND result "${char}")
            math(EXPR curr "${curr} + 1")
        endif()
    endwhile()

    if(ARG_RESULT)
        set(${ARG_RESULT} ${result} PARENT_SCOPE)
    endif()

    if(message_type_length EQUAL 1)
        message(${message_type} ${result})
    endif()
endfunction()
