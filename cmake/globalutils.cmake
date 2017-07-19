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
# Joins list by replacing the separator with glue
function(join sep glue output)
    string (REGEX REPLACE "([^\\]|^)${sep}" "\\1${glue}" _TMP_STR "${ARGN}")
    # duplicate all backslashes
    string (REGEX REPLACE "[\\](.)" "\\1" _TMP_STR "${_TMP_STR}")
    set(${output} "${_TMP_STR}" PARENT_SCOPE)
endfunction()

#--------------------------------------------------------------------
# ivw_prepend(output str)
# ivw_prepends str to each element of the input
function(ivw_prepend var prefix)
   set(listVar "")
   foreach(f ${ARGN})
      list(APPEND listVar "${prefix}${f}")
   endforeach(f)
   set(${var} "${listVar}" PARENT_SCOPE)
endfunction(ivw_prepend)

#--------------------------------------------------------------------
# encodeLineBreaks(output strings)
# encodes the contents of the string given as last argument and saves the 
# result in output.
# Linebreaks ('\n') and semicolon (';') are replaced for better handling 
# within CMAKE with __LINEBREAK__ and __SEMICOLON__, respectively.
function(encodeLineBreaks output)
    # replace linebreaks
    string(REPLACE "\n" "__LINEBREAK__" _tmp_str "${ARGN}")
    # replace semicolon, as it is interpreted as a list separator by CMAKE
    string(REPLACE ";" "__SEMICOLON__" _tmp_str "${_tmp_str}")
    set(${output} "${_tmp_str}" PARENT_SCOPE)
endfunction()

#--------------------------------------------------------------------
# decodeLineBreaks(output strings)
# reverse the encoding done by encodeLineBreaks(), i.e. __LINEBREAK__ and 
# __SEMICOLON__ are reverted to '\n' and ';', respectively.
function(decodeLineBreaks output)
    # revert linebreaks
    string(REPLACE "__LINEBREAK__" "\n" _tmp_str "${ARGN}")
    # revert semicolon
    string(REPLACE "__SEMICOLON__" ";" _tmp_str "${_tmp_str}")
    set(${output} "${_tmp_str}" PARENT_SCOPE)
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
        decodeLineBreaks(decodedDesc ${${mod_dep}_description})
        set(desc "Build ${dir_name_cap} Module\n${decodedDesc}")
    else()
        set(desc "Build ${dir_name_cap} Module")
    endif()

    if(forcemodule)
        set(${mod_name} ${onoff} CACHE BOOL "${desc}" FORCE)
    elseif(NOT DEFINED ${mod_name})
        option(${mod_name} "${desc}" ${onoff})
    else()
        # need to do this to update the docstring
        set(${mod_name} ${${mod_name}} CACHE BOOL "${desc}" FORCE)
    endif()
endfunction()

#--------------------------------------------------------------------
# Name conventions:
# opengl               : dir                : Name of module folder, should be lowercase with no spaces (opengl)
# OPENGL               : macro_name         : C Macro name, to uppercase, "-" -> "_"
# INVIWOOPENGLMODULE   : mod_dep            : Internal name for module all uppercase
# IVW_MODULE_OPENGL    : mod_prefix         : Name of cmake option for module
# InviwoOpenGLModule   : mod_name           : The name of a module same as mod_dep, but not uppercase
# inviwo-module-opengl : module_target_name : The name of the target for a module
# 
# Name conversion functions:
# ivw_to_macro_name            OpenGL-test        -> OPENGL_TEST
# ivw_dir_to_mod_dep           OpenGL             -> INVIWOOPENGLMODULE
# ivw_mod_dep_to_dir           INVIWOOPENGLMODULE -> opengl 
# ivw_dir_to_mod_prefix        OpenGL             -> IVW_MODULE_OPENGL
# ivw_mod_prefix_to_dir        IVW_MODULE_OPENGL  -> opengl
# ivw_mod_name_to_dir          InviwoOpenGLModule -> opengl
# ivw_mod_name_to_mod_dep      InviwoOpenGLModule -> INVIWOOPENGLMODULE
# ivw_to_mod_name              OpenGL             -> InviwoOpenGLModule
# ivw_dir_to_module_taget_name opengl             -> inviwo-module-opengl
#--------------------------------------------------------------------

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
# Convert directory name tp module dep, i.e. opengl -> INVIWOOPENGLMODULE
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
# Convert dir name to  module prefix, i.e. opengl -> IVW_MODULE_OPENGL
function(ivw_dir_to_mod_prefix retval)
    set(the_list "")
    foreach(item ${ARGN})
        string(TOUPPER ${item} u_item)
        list(APPEND the_list IVW_MODULE_${u_item})
    endforeach()
    set(${retval} ${the_list} PARENT_SCOPE)
endfunction()

#--------------------------------------------------------------------
# ivw_mod_dep_to_dir(relval item1 item2 ...)
# Convert module dep to directory name, i.e. INVIWOOPENGLMODULE -> opengl
function(ivw_mod_dep_to_dir retval)
    set(the_list "")
    foreach(item ${ARGN})
        string(REGEX MATCH "^INVIWO(.*)MODULE$" found_item ${item})
        if(CMAKE_MATCH_1)
            string(TOLOWER ${CMAKE_MATCH_1} l_new_item)
            list(APPEND the_list ${l_new_item})
        endif()
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

#--------------------------------------------------------------------
# ivw_mod_name_to_mod_dep(retval item1 item2 ...)
# Convert module name to module dep, i.e. opengl -> INVIWOOPENGLMODULE
function(ivw_mod_name_to_mod_dep retval)
    set(the_list "")
    foreach(item ${ARGN})
        string(TOUPPER ${item} uitem)
        list(APPEND the_list ${uitem})
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

#--------------------------------------------------------------------
# pritty print a list .
# ivw_print_list(list) -> "list  = list1, list2, list3"
function(ivw_print_list list_var)
    string(REPLACE ";" ", " res "${${list_var}}") 
    ivw_message("${list_var} = ${res}")
endfunction()

#--------------------------------------------------------------------
# repeat a string n times.
# ivw_repeat_str("-" 10 ret) -> "----------"
function(ivw_repeat_str str n retval)
    set(res "")
    while(${n} GREATER 0)
        set(res "${res}${str}")
        MATH(EXPR n "${n}-1")
    endwhile()
    set(${retval} ${res} PARENT_SCOPE)
endfunction()

#--------------------------------------------------------------------
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

#--------------------------------------------------------------------
# helper function for topological sort
function(ivw_private_visit_node node sorted_var marked_var tempmarked_var node_list_var node_edge_var count)
    MATH(EXPR count "${count}+1")
    if(${count} GREATER 30)
        ivw_message(ERROR "Stoppig to deep recursion")
        return()
    endif()

    set(sorted ${${sorted_var}})
    set(marked ${${marked_var}})
    set(tempmarked ${${tempmarked_var}})
    set(node_list ${${node_list_var}})

    list(FIND tempmarked ${node} tempfound)
    if(NOT ${tempfound} EQUAL -1) # Error not a dag
        ivw_message(ERROR "Dependency graph not a DAG. Cant resove for node \"${node}\"")
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

#--------------------------------------------------------------------
# Get the module name from a CMakeLists.txt
function(ivw_private_get_ivw_module_name path retval)
    file(READ ${path} contents)
     string(REPLACE "\n" ";" lines "${contents}")
     foreach(line ${lines})
        #\s*ivw_module\(\s*(\w+)\s*\)\s*
        string(REGEX MATCH "\\s*ivw_module\\(\\s*([A-Za-z0-9_-]+)\\s*\\)\\s*" found_item ${line})
        if(CMAKE_MATCH_1)
            set(${retval} ${CMAKE_MATCH_1} PARENT_SCOPE)
            return()
       endif()
     endforeach()
     set(${retval} NOTFOUND PARENT_SCOPE)
endfunction()


#--------------------------------------------------------------------
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

#--------------------------------------------------------------------
# From a 
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


#--------------------------------------------------------------------
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

#--------------------------------------------------------------------
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


#--------------------------------------------------------------------
# A helper funtion to install targets
function(ivw_default_install_targets)
    # package the zlib lib
    if(IVW_PACKAGE_PROJECT AND BUILD_SHARED_LIBS)  
        if(WIN32)
           install(TARGETS ${ARGN}
                    RUNTIME DESTINATION bin
                    COMPONENT modules)
        elseif(APPLE)
            install(TARGETS ${ARGN}
                    RUNTIME DESTINATION bin
                    BUNDLE DESTINATION .
                    ARCHIVE DESTINATION Inviwo.app/Contents/MacOS
                    LIBRARY DESTINATION Inviwo.app/Contents/MacOS
                    COMPONENT modules)
        
        else()
            install(TARGETS ${ARGN}
                    RUNTIME DESTINATION bin
                    BUNDLE DESTINATION bin
                    ARCHIVE DESTINATION lib
                    LIBRARY DESTINATION lib
                    COMPONENT modules)
        endif()
    endif()
endfunction()

