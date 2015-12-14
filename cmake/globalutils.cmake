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
# This file contains a collection of CMake utility scripts

#--------------------------------------------------------------------
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
    foreach(item ${ARGN})
        string(TOLOWER ${item} lowercase)
        list(APPEND the_list ${lowercase})
    endforeach()
    set(${retval} ${the_list} PARENT_SCOPE)
endfunction()

#--------------------------------------------------------------------
# join(sep glue output)
# Joins list
function(join sep glue output)
    string (REGEX REPLACE "([^\\]|^)${sep}" "\\1${glue}" _TMP_STR "${ARGN}")
    string (REGEX REPLACE "[\\](.)" "\\1" _TMP_STR "${_TMP_STR}")
    set(${output} "${_TMP_STR}" PARENT_SCOPE)
endfunction()

#--------------------------------------------------------------------
# list_all_variables()
# prints all variables and values.
macro(list_all_variables)
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
endmacro()

#--------------------------------------------------------------------
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

#--------------------------------------------------------------------
# remove_duplicates(retval item item ...)
# Clean duplicates from list subdirectories
function(remove_duplicates retval)
    set(list_of_dirs ${ARGN})
    if(list_of_dirs)
        list(REMOVE_DUPLICATES list_of_dirs)
    endif()
    set(${retval} ${list_of_dirs} PARENT_SCOPE)
endfunction()

#--------------------------------------------------------------------
# remove_from_list(retval thelist toRemove0 toRemove1 ...)
# Remove entries in one list from another list
function(remove_from_list retval thelist)
    set(new_items ${thelist})
    set(old_items ${ARGN})
    if(old_items AND new_items)
        foreach(item ${old_items})
            list(REMOVE_ITEM new_items ${item})
        endforeach()
    endif()
    set(${retval} ${new_items} PARENT_SCOPE)
endfunction()


#--------------------------------------------------------------------
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

#--------------------------------------------------------------------
# list_to_stringvector(retval item1 item2 ...) -> {"item1", "item2", ...}
# builds a string vector 
function(list_to_stringvector retval)
    set (items ${ARGN})
    list(LENGTH items len)
    if (${len} GREATER 0)
        join(";" "\", \"" res ${items})
        set(${retval} "{\"${res}\"}" PARENT_SCOPE)
    else()
        set(${retval} "{}" PARENT_SCOPE)
    endif()
endfunction()
function(list_to_longstringvector retval) # same but with linebreaks
    set (items ${ARGN})
    list(LENGTH items len)
    if (${len} GREATER 0)
        join(";" "\",\n    \"" res ${items})
        set(${retval} "{\n    \"${res}\"\n}" PARENT_SCOPE)
    else()
        set(${retval} "{}" PARENT_SCOPE)
    endif()
endfunction()

#--------------------------------------------------------------------
# mark_as_internal(var)
# hides the given variable in all CMake UIs
macro ( mark_as_internal _var )
  set ( ${_var} ${${_var}} CACHE INTERNAL "hide this!" FORCE )
endmacro( mark_as_internal _var )

#--------------------------------------------------------------------
# ivw_add_module_option_to_cache(module_dir onoff force)
# add/update a module to the CMake cache
function(ivw_add_module_option_to_cache the_module onoff forcemodule)
    ivw_dir_to_mod_prefix(mod_name ${the_module})
    ivw_dir_to_mod_dep(mod_dep ${the_module})
    first_case_upper(dir_name_cap ${the_module})

    if(${mod_dep}_description)
        set(desc "Build ${dir_name_cap} Module\n${${mod_dep}_description}")
    else()
        set(desc "Build ${dir_name_cap} Module")
    endif()

    if(forcemodule)
        set(${mod_name} ${onoff} CACHE BOOL "${desc}" FORCE)
    elseif(NOT DEFINED ${mod_name})
        option(${mod_name} "${desc}" ${onoff})
    endif()
endfunction()

#--------------------------------------------------------------------
# ivw_to_macro_name(retval item1 item2 ...)
# Convert a name to a macro name, i.e. OpenGL-test -> OPENGL_TEST
function(ivw_to_macro_name retval)
    set(the_list "")
    foreach(item ${ARGN})
        string(TOUPPER ${item} u_item)
        string(REGEX REPLACE "-" "_" new_item ${u_item})
        list(APPEND the_list "${new_item}")
    endforeach()
    set(${retval} ${the_list} PARENT_SCOPE)
endfunction()


#--------------------------------------------------------------------
# ivw_dir_to_mod_dep(retval item1 item2 ...)
# Convert module prefix to directory name, i.e. OpenGL -> INVIWOOPENGLMODULE
function(ivw_dir_to_mod_dep retval)
    set(the_list "")
    foreach(item ${ARGN})
        string(TOUPPER ${item} u_item)
        list(APPEND the_list "INVIWO${u_item}MODULE")
    endforeach()
    set(${retval} ${the_list} PARENT_SCOPE)
endfunction()


#--------------------------------------------------------------------
# ivw_dir_to_mod_prefix(retval item1 item2 ...)
# Convert module prefix to directory name, i.e. OpenGL -> IVW_MODULE_OPENGL
function(ivw_dir_to_mod_prefix retval)
    set(the_list "")
    foreach(item ${ARGN})
        string(TOUPPER ${item} u_item)
        list(APPEND the_list IVW_MODULE_${u_item})
    endforeach()
    set(${retval} ${the_list} PARENT_SCOPE)
endfunction()

#--------------------------------------------------------------------
# ivw_mod_prefix_to_dir(relval item1 item2 ...)
# Convert module prefix to directory name, i.e. IVW_MODULE_OPENGL -> opengl
function(ivw_mod_prefix_to_dir retval)
    set(the_list "")
    foreach(item ${ARGN})
        string(REGEX MATCH "(^IVW_MODULE_$)" found_item ${item})
        if(found_item)
            string(REGEX REPLACE "(^IVW_MODULE_$)" "" new_item ${item})
            string(TOLOWER ${new_item} l_new_item)
            list(APPEND the_list ${l_new_item})
        endif()
    endforeach()
    set(${retval} ${the_list} PARENT_SCOPE)
endfunction()

#--------------------------------------------------------------------
# ivw_mod_name_to_dir(retval item1 item2 ...)
# Convert module name to directory name, i.e. InviwoOpenGLModule -> opengl
function(ivw_mod_name_to_dir retval)
    set(the_list "")
    foreach(item ${ARGN})
        string(REGEX MATCH "(^Inviwo.*.Module$)" found_item ${item})
        if(found_item)
            string(REGEX REPLACE "(^Inviwo)|(Module$)" "" new_item ${item})
            string(TOLOWER ${new_item} l_new_item)
            list(APPEND the_list ${l_new_item})
        endif()
    endforeach()
    set(${retval} ${the_list} PARENT_SCOPE)
endfunction()

#--------------------------------------------------------------------
# ivw_to_mod_name(retval item1 item2 ...)
# Convert module name to directory name, i.e. OpenGL -> InviwoOpenGLModule
function(ivw_to_mod_name retval)
    set(the_list "")
    foreach(item ${ARGN})
        list(APPEND the_list Inviwo${item}Module)
    endforeach()
    set(${retval} ${the_list} PARENT_SCOPE)
endfunction()


#--------------------------------------------------------------------
# ivw_dir_to_module_taget_name(retval item1 item2 ...)
# Convert module name to directory name, i.e. opengl -> inviwo-module-opengl
function(ivw_dir_to_module_taget_name retval)
    set(the_list "")
    foreach(item ${ARGN})
        list(APPEND the_list inviwo-module-${item})
    endforeach()
    set(${retval} ${the_list} PARENT_SCOPE)
endfunction()