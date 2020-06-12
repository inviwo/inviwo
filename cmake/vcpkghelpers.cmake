#################################################################################
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2020 Inviwo Foundation
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

function(ivw_vcpkg_install name)
	if(NOT VCPKG_TOOLCHAIN)
		return()
	endif()

    set(options "")
    set(oneValueArgs COPYRIGHT VERSION)
    set(multiValueArgs "")
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	string(TOLOWER "${name}" lowercase_name)
	set(pkgdir "${_VCPKG_ROOT_DIR}/packages/${lowercase_name}_${VCPKG_TARGET_TRIPLET}")
    
    install(
    	DIRECTORY "${pkgdir}/bin" 
    	DESTINATION ${IVW_LIBRARY_INSTALL_DIR}
    	PATTERN "*.dll"
    )

    if(ARG_VERSION)
        set(version "?.?.?")
        if(EXISTS ${pkgdir}/CONTROL)
            file(STRINGS ${pkgdir}/CONTROL lines)
            foreach(line IN LISTS lines)
                if(line MATCHES "Version: (.*)")
                    set(version ${CMAKE_MATCH_1})
                endif()
            endforeach()
        endif()
        set(${ARG_VERSION} ${version} PARENT_SCOPE)
    endif()

    if(ARG_COPYRIGHT)
        set(copyright "${pkgdir}/share/${lowercase_name}/copyright")
        set(${ARG_COPYRIGHT} ${copyright} PARENT_SCOPE)
    endif()
endfunction()
