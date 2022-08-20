#################################################################################
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2020-2022 Inviwo Foundation
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



if(WIN32)
    option(IVW_CFG_PRECOMPILED_HEADERS "Create and use precompilied headers" ON)
else()
    option(IVW_CFG_PRECOMPILED_HEADERS "Create and use precompilied headers" OFF)
endif()


if(IVW_CFG_PRECOMPILED_HEADERS)
    include(${CMAKE_CURRENT_LIST_DIR}/cotire.cmake)
endif()



# Set header ignore paths for cotire

function(ivw_get_header_path header retval)
    file(WRITE "${IVW_BINARY_DIR}/cmake/findheader.cpp" "#include <${header}>")
    string(TOLOWER "${header}" lheader)

    set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
    set(CMAKE_TRY_COMPILE_CONFIGURATION RELEASE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /showIncludes")
    try_compile(res ${CMAKE_BINARY_DIR}
        SOURCES "${IVW_BINARY_DIR}/cmake/findheader.cpp"
        OUTPUT_VARIABLE output
    )
    if(NOT ${res})
        message(FATAL_ERROR "Header path not found")
    endif()
    string (REPLACE "\\" "/" output "${output}")
    string (REPLACE "//" "/" output "${output}")
    string (REPLACE ";" "\\;" output "${output}")
    string (REGEX REPLACE "\n" ";" output "${output}")
    string (REGEX REPLACE "\r" "" output "${output}")
    foreach(line ${output})
        if (line MATCHES ":( +)([^:]+:[^:]+)$")
            string (LENGTH "${CMAKE_MATCH_1}" depth)
            get_filename_component(file "${CMAKE_MATCH_2}" ABSOLUTE)
            get_filename_component(name "${file}" NAME)
            string(TOLOWER "${name}" lname)
            if(${lname} STREQUAL ${lheader})
                get_filename_component(path ${file} DIRECTORY)
                set(${retval} ${path} PARENT_SCOPE)
                return()
            endif()
        endif()
    endforeach()
    message(FATAL_ERROR "Header path not found")
endfunction()

function(ivw_get_drive file retval)
    set(tmp1 ${file})
    set(tmp2 "")
    while(NOT "${tmp1}" STREQUAL "${tmp2}")
        set(tmp2 ${tmp1})
        get_filename_component(tmp1 ${tmp1} DIRECTORY)
    endwhile()
    set(${retval} ${tmp1} PARENT_SCOPE)
endfunction()

if(WIN32 AND MSVC AND IVW_CFG_PRECOMPILED_HEADERS)
    ivw_get_header_path("windows.h" ivw_private_windows_path)
endif()

# Optimize compilation with pre-compilied headers from inviwo core
function(ivw_compile_optimize_inviwo_core_on_target target)
    message(DEPRECATION "Use ivw_compile_optimize_on_target")
    ivw_compile_optimize_on_target(${target})
endfunction()

# Optimize compilation with pre-compilied headers
# Custom target properties:
#  * COTIRE_PREFIX_HEADER_PUBLIC_IGNORE_PATH  
#  * COTIRE_PREFIX_HEADER_PUBLIC_INCLUDE_PATH 
# We make sure that these properties are propagated to the 
# depending targets.
function(ivw_compile_optimize_on_target target)
    if(IVW_CFG_PRECOMPILED_HEADERS)
        ivw_get_target_property_recursive(publicIgnorePaths ${target} COTIRE_PREFIX_HEADER_PUBLIC_IGNORE_PATH False)
        get_target_property(ignorePaths ${target} COTIRE_PREFIX_HEADER_IGNORE_PATH)
        if(NOT ignorePaths)
            set(ignorePaths "")
        endif()
        list(APPEND ignorePaths
            ${publicIgnorePaths}
            "${CMAKE_CURRENT_SOURCE_DIR}"
            "${CMAKE_CURRENT_BINARY_DIR}"
            "${IVW_EXTENSIONS_DIR}/warn"
        )
        if(WIN32 AND MSVC)
            ivw_get_drive(${ivw_private_windows_path} windrive)
            list(APPEND ignorePaths ${windrive})
        endif()
        list(REMOVE_DUPLICATES ignorePaths)

        ivw_get_target_property_recursive(publicIncludePaths ${target} COTIRE_PREFIX_HEADER_PUBLIC_INCLUDE_PATH False)
        get_target_property(includePaths ${target} COTIRE_PREFIX_HEADER_INCLUDE_PATH)
        if(NOT includePaths)
            set(includePaths "")
        endif()
        list(APPEND includePaths
            "${IVW_ROOT_DIR}"
            ${publicIncludePaths}
        )
        list(REMOVE_DUPLICATES includePaths)

        set_target_properties(${target} PROPERTIES COTIRE_PREFIX_HEADER_IGNORE_PATH "${ignorePaths}")
        set_target_properties(${target} PROPERTIES COTIRE_PREFIX_HEADER_INCLUDE_PATH "${includePaths}")
        set_target_properties(${target} PROPERTIES COTIRE_ADD_UNITY_BUILD FALSE)

        cotire(${target})
    endif()
endfunction()
