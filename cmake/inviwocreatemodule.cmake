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

# Creates an Inviwo module
macro(ivw_module project_name)
    project(${project_name} ${ARGN})
endmacro()

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
        ${${mod}_sharedLibHpp}
        ${${mod}_sharedLibCpp}
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

    ivw_register_docs(INPUT_DIRS ${${mod}_incPath}
        IMAGE_PATHS ${${mod}_path}/docs/images
        INCLUDE_PATHS ${${mod}_path}/glsl
        INCLUDE_PATHS_FROM_TARGETS ${${mod}_target}
    )

    set_target_properties(${${mod}_target} PROPERTIES VERSION ${${mod}_version})
endfunction()