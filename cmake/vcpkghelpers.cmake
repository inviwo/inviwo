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

function(ivw_vcpkg_paths)
    set(options "")
    set(oneValueArgs BIN INCLUDE LIB SHARE)
    set(multiValueArgs "")
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT VCPKG_TOOLCHAIN)
        if(ARG_BIN)
            set(${ARG_BIN} "" PARENT_SCOPE)
        endif()
        if(ARG_INCLUDE)
            set(${ARG_INCLUDE} "" PARENT_SCOPE)
        endif()
        if(ARG_LIB)
            set(${ARG_LIB} "" PARENT_SCOPE)
        endif()
        if(ARG_SHARE)
            set(${ARG_SHARE} "" PARENT_SCOPE)
        endif()
        return()
    endif()

    if(ARG_BIN)
        set(${ARG_BIN} "${_VCPKG_ROOT_DIR}/installed/${VCPKG_TARGET_TRIPLET}/bin" PARENT_SCOPE)
    endif()
    if(ARG_INCLUDE)
        set(${ARG_INCLUDE} "${_VCPKG_ROOT_DIR}/installed/${VCPKG_TARGET_TRIPLET}/include" PARENT_SCOPE)
    endif()
    if(ARG_LIB)
        set(${ARG_LIB} "${_VCPKG_ROOT_DIR}/installed/${VCPKG_TARGET_TRIPLET}/lib" PARENT_SCOPE)
    endif()
    if(ARG_SHARE)
        set(${ARG_SHARE} "${_VCPKG_ROOT_DIR}/installed/${VCPKG_TARGET_TRIPLET}/share" PARENT_SCOPE)
    endif()
endfunction()

function(ivw_vcpkg_install name)
	if(NOT VCPKG_TOOLCHAIN)
		return()
	endif()

    set(options EXT)
    set(oneValueArgs OUT_COPYRIGHT OUT_VERSION COPYRIGHT MODULE)
    set(multiValueArgs "")
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	string(TOLOWER "${name}" lowercase_name)
	set(pkgdir "${_VCPKG_ROOT_DIR}/packages/${lowercase_name}_${VCPKG_TARGET_TRIPLET}")
    set(portdir "${_VCPKG_ROOT_DIR}/ports/${lowercase_name}")

    if(EXISTS "${pkgdir}/bin")
        install(
    	   DIRECTORY "${pkgdir}/bin/" 
    	   DESTINATION ${IVW_RUNTIME_INSTALL_DIR}
           COMPONENT Application
    	   FILES_MATCHING PATTERN "*${CMAKE_SHARED_LIBRARY_SUFFIX}"
        )
        install(
           DIRECTORY "${pkgdir}/bin/" 
           DESTINATION ${IVW_RUNTIME_INSTALL_DIR}
           COMPONENT Development
           FILES_MATCHING PATTERN "*.pdb"
        )
        install(
           DIRECTORY "${pkgdir}/lib/" 
           DESTINATION ${IVW_LIBRARY_INSTALL_DIR}
           COMPONENT Development
           FILES_MATCHING PATTERN "*${CMAKE_LINK_LIBRARY_SUFFIX}"
        )
    endif()

    set(version "?.?.?")
    set(depends "")
    if(EXISTS ${pkgdir}/CONTROL)
        file(STRINGS ${pkgdir}/CONTROL lines)
        foreach(line IN LISTS lines)
            if(line MATCHES "Version: (.*)")
                set(version ${CMAKE_MATCH_1})
            endif()
            if(line MATCHES "Depends: (.*)")
                set(depDepends ${CMAKE_MATCH_1})
                string(REPLACE "," ";" depDepends ${depDepends})
                list(TRANSFORM depDepends STRIP)
                list(APPEND depends ${depDepends})
            endif()
        endforeach()
    else()
        message(WARNING "Vcpkg control file not found ${pkgdir}/CONTROL")
    endif()

    set(homepage "")
    if(EXISTS ${portdir}/CONTROL)
        file(STRINGS ${portdir}/CONTROL lines)
        foreach(line IN LISTS lines)
            if(line MATCHES "Homepage: (.*)")
                set(homepage URL ${CMAKE_MATCH_1})
            endif()
        endforeach()
    else()
        message(WARNING "Vcpkg control file not found ${portdir}/CONTROL")
    endif()    

    if(ARG_COPYRIGHT)
        set(copyright "${pkgdir}/share/${lowercase_name}/${ARG_COPYRIGHT}")
    else()
        set(copyright "${pkgdir}/share/${lowercase_name}/copyright")
    endif()

    if(ARG_EXT) 
        set(ext EXT)
    else()
        set(ext "")
    endif()

    if(EXISTS ${copyright})
        ivw_register_license_file(NAME ${name} 
            VERSION ${version} MODULE ${ARG_MODULE} ${ext}
            ${homepage} FILES ${copyright}
        )
    endif()

    foreach(dep IN LISTS depends)
        ivw_vcpkg_install(${dep} MODULE ${ARG_MODULE} ${ext})
    endforeach()
    
    if(ARG_OUT_VERSION)
        set(${ARG_OUT_VERSION} ${version} PARENT_SCOPE)
    endif()
    if(ARG_OUT_COPYRIGHT)
        set(${ARG_OUT_COPYRIGHT} ${copyright} PARENT_SCOPE)
    endif()
endfunction()
