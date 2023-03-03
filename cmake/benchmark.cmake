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

function(ivw_benchmark)
    set(options "")
    set(oneValueArgs "NAME")
    set(multiValueArgs "FILES;LIBS")
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    ivw_group("Source Files" ${ARG_FILES})

    # Create application
    add_executable(${ARG_NAME} ${ARG_FILES})
    find_package(benchmark CONFIG REQUIRED)
    target_link_libraries(${ARG_NAME} PUBLIC  benchmark::benchmark ${ARG_LIBS})
    set_target_properties(${ARG_NAME} PROPERTIES FOLDER benchmarks)

    if(MSVC)
        set_property(TARGET ${ARG_NAME} APPEND_STRING PROPERTY LINK_FLAGS 
            " /SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup")
    endif()

    # Define defintions and properties
    ivw_define_standard_properties(${ARG_NAME})
    ivw_define_standard_definitions(${ARG_NAME} ${ARG_NAME})
endfunction()





