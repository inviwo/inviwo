#################################################################################
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2020-2025 Inviwo Foundation
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

option(IVW_CFG_PRECOMPILED_HEADERS "Enable precompiled headers (PCH)" OFF)

define_property(
    TARGET PROPERTY IVW_PCH_HEADERS 
    INHERITED
    BRIEF_DOCS "List of headers used for PCH in this target"
    FULL_DOCS "List of headers used for PCH in this target"
)

function(ivw_compile_optimize_inviwo_core_on_target target)
    message(DEPRECATION "Use ivw_compile_optimize_on_target")
    ivw_compile_optimize_on_target(${target})
endfunction()

#--------------------------------------------------------------------
# Optimize compilation by reusing pre-compiled headers
# The HEADERS parameter is used to create custom PCH files for the given target. The headers are 
# also stored in the IVW_PCH_HEADERS target property.
# If no headers are present, the PCH files from either inviwo-core or inviwo-module-qtwidgets will 
# be reused (based on the target's dependencies).
function(ivw_compile_optimize_on_target target)
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs "HEADERS")
    cmake_parse_arguments(PARSE_ARGV 1 ARG "${options}" "${oneValueArgs}" "${multiValueArgs}")

    if(IVW_CFG_PRECOMPILED_HEADERS)
        if(ARG_HEADERS)
            ivw_get_target_property_recursive(propval ${target} IVW_PCH_HEADERS TRUE)
            set_property(TARGET ${target} PROPERTY IVW_PCH_HEADERS ${propval} ${ARG_HEADERS})
            target_precompile_headers(${target} PRIVATE ${propval} ${ARG_HEADERS})
        else()
            ivw_get_target_property_recursive(dependencies ${target} NAME TRUE)
            if ("${dependencies}" MATCHES "inviwo-module-qtwidgets")
                ivw_create_pch_dummy(inviwo-module-qtwidgets)
                target_precompile_headers(${target} REUSE_FROM inviwo-module-qtwidgets-pch)
            elseif ("${dependencies}" MATCHES "inviwo-core")
                ivw_create_pch_dummy(inviwo-core)
                target_precompile_headers(${target} REUSE_FROM inviwo-core-pch)
            endif()
        endif()
    endif()
endfunction()

#--------------------------------------------------------------------
# Creates PCH dummy target for target
# The name of the dummy target is '${target}-pch'.
# A dummy target is necessary when reusing PCH files in builds with dynamic linking due to 
# dllimport/dllexport definitions which need to match between PCH and regular includes.
# Therefore we cannot simply reuse the PCH of inviwo-core since its members are exported 
# whereas in the dummy inviwo-core-pch they are imported.
function(ivw_create_pch_dummy target)
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs "HEADERS")
    cmake_parse_arguments(PARSE_ARGV 1 ARG "${options}" "${oneValueArgs}" "${multiValueArgs}")

    if(TARGET ${target}-pch)
        # PCH dummy target already exists
        return()
    endif()

    if(TARGET ${target})
        ivw_get_target_property_recursive(propval ${target} IVW_PCH_HEADERS TRUE)
        if(propval)
            list(PREPEND ARG_HEADERS ${propval})
        endif()
    endif()

    if(NOT ARG_HEADERS)
        message(AUTHOR_WARNING "No headers provided, skipping PCH generation for ${target}-pch.")
        return()
    endif()

    set(PCH_DUMMY_CPP_FILE ${CMAKE_BINARY_DIR}/pch/pch-dummy-${target}.cpp)
    file(WRITE ${PCH_DUMMY_CPP_FILE} 
        "// Dummy source file needed in target for reusing pch artifacts\n"
    )
    add_library(${target}-pch STATIC ${PCH_DUMMY_CPP_FILE})
    # ensure same definitions and properties as used for modules
    ivw_define_standard_definitions(${target}-pch ${target}-pch)
    ivw_define_standard_properties(${target}-pch)
    target_link_libraries(${target}-pch PRIVATE ${target})
    # make PCH private, otherwise it is not possible to disable PCH in any child projects 
    target_precompile_headers(${target}-pch PRIVATE ${ARG_HEADERS})

    ivw_folder(${target}-pch "pch-targets")
endfunction()