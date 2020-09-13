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

# A helper function to get to various vcpkg paths
# If vcpkg is not used we just return empty strings
# There are not "offically" exposed so we always use this helper to get then if needed
# then we only need to update here if vcpkg changes. 
# This is mostly used to provide hints when trying to find headers etc for libs that don't have
# a proper FindFoo.cmake of FooConfig.cmake.
function(ivw_vcpkg_paths)
    set(options "")
    set(oneValueArgs BIN INCLUDE LIB SHARE BASE)
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
        if(ARG_BASE)
            set(${ARG_BASE} "" PARENT_SCOPE)
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
    if(ARG_BASE)
        set(${ARG_SHARE} "${_VCPKG_ROOT_DIR}/installed/${VCPKG_TARGET_TRIPLET}" PARENT_SCOPE)
    endif()
endfunction()

if(VCPKG_TOOLCHAIN)
    ivw_git_get_hash(${_VCPKG_ROOT_DIR} ivw_vcpkg_sha)
endif()

# A helper function to install vcpkg libs. Will install dll/so, lib, pdb, stc. into the 
# correspnding folders by globing the vcpkg package folders. 
# It will also try and install eny transitive dependencies autoamtically 
# We also autoamtically register the licence file using the metadata found in vcpkg
# Params:
#  * OUT_COPYRIGHT retrive the path the the COPYRIGHT file
#  * OUT_VERSION get the package version from the vcpkg metadata
#  * MODULE the module to use for ivw_register_license_file
#  * EXT pass on to ivw_register_license_file
function(ivw_vcpkg_install name)
	if(NOT VCPKG_TOOLCHAIN)
		return()
	endif()

    set(options EXT)
    set(oneValueArgs OUT_COPYRIGHT OUT_VERSION MODULE)
    set(multiValueArgs "")
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	string(TOLOWER "${name}" lowercase_name)

    if(DEFINED IVW_CFG_VCPKG_OVERLAYS)
        set(overlay --overlay ${IVW_CFG_VCPKG_OVERLAYS})
    else()
        set(overlay "")
    endif()

    if(NOT DEFINED ivw_vcpkg_info_${lowercase_name} OR 
        NOT ivw_vcpkg_info_${lowercase_name}_sha STREQUAL ivw_vcpkg_sha)
        message(STATUS "Vcpkg fetching metadata for: ${name}")
        execute_process(
            COMMAND "${PYTHON_EXECUTABLE}" "${IVW_TOOLS_DIR}/vcpkginfo.py"
                --vcpkg "${_VCPKG_EXECUTABLE}" 
                ${overlay}
                --pkg ${lowercase_name}
                --triplet ${VCPKG_TARGET_TRIPLET}
            OUTPUT_VARIABLE pkgInfo
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(NOT pkgInfo)
            message(WARNING "Unable to retrive vcpkg package info for ${name}")
        else()
            set("ivw_vcpkg_info_${lowercase_name}" "${pkgInfo}" CACHE INTERNAL "Vcpkg meta data")
            set("ivw_vcpkg_info_${lowercase_name}_sha" "${ivw_vcpkg_sha}" CACHE INTERNAL "Vcpkg SHA")
        endif()
    else()
        set(pkgInfo ${ivw_vcpkg_info_${lowercase_name}})
    endif()

    set(vcoptions "")
    set(vconeValueArgs VCPKG_VERSION VCPKG_HOMEPAGE VCPKG_DESCRIPTION)
    set(vcmultiValueArgs VCPKG_DEPENDENCIES VCPKG_OWNED_FILES)
    cmake_parse_arguments(INFO "${vcoptions}" "${vconeValueArgs}" "${vcmultiValueArgs}" ${pkgInfo})

    set(binfiles ${INFO_VCPKG_OWNED_FILES})
    string(REPLACE "." "\\." binsuffix ${CMAKE_SHARED_LIBRARY_SUFFIX})
    list(FILTER binfiles INCLUDE REGEX "${VCPKG_TARGET_TRIPLET}/bin/.*${binsuffix}") 
    list(TRANSFORM binfiles PREPEND "${_VCPKG_ROOT_DIR}/installed/")

    set(pdbfiles ${INFO_VCPKG_OWNED_FILES})
    list(FILTER pdbfiles INCLUDE REGEX "${VCPKG_TARGET_TRIPLET}/bin/.*\\.pdb")
    list(TRANSFORM pdbfiles PREPEND "${_VCPKG_ROOT_DIR}/installed/")

    set(libfiles ${INFO_VCPKG_OWNED_FILES})
    string(REPLACE "." "\\." libsuffix ${CMAKE_LINK_LIBRARY_SUFFIX})
    list(FILTER libfiles INCLUDE REGEX "${VCPKG_TARGET_TRIPLET}/lib/.*${libsuffix}")
    list(TRANSFORM libfiles PREPEND "${_VCPKG_ROOT_DIR}/installed/")

    set(copyright ${INFO_VCPKG_OWNED_FILES})
    list(FILTER copyright INCLUDE REGEX "${VCPKG_TARGET_TRIPLET}/share/.*copyright.*")
    list(TRANSFORM copyright PREPEND "${_VCPKG_ROOT_DIR}/installed/")

    set(headers ${INFO_VCPKG_OWNED_FILES})
    list(FILTER headers INCLUDE REGEX "${VCPKG_TARGET_TRIPLET}/include/.*\\..?.*")
    list(TRANSFORM headers PREPEND "${_VCPKG_ROOT_DIR}/installed/")

    install(
       FILES ${binfiles} 
       DESTINATION ${IVW_RUNTIME_INSTALL_DIR}
       COMPONENT Application
    )
    install(
       FILES ${pdbfiles} 
       DESTINATION ${IVW_RUNTIME_INSTALL_DIR}
       COMPONENT Development
    )
    install(
       FILES ${libfiles} 
       DESTINATION ${IVW_LIBRARY_INSTALL_DIR}
       COMPONENT Development
    )

    if(INFO_VCPKG_HOMEPAGE)
        set(homepage URL ${INFO_VCPKG_HOMEPAGE})
    else()
        set(homepage "")
    endif()

    if(ARG_EXT) 
        set(ext EXT)
    else()
        set(ext "")
    endif()

    if(copyright)
        ivw_register_license_file(NAME ${name} 
            VERSION ${INFO_VCPKG_VERSION} MODULE ${ARG_MODULE} ${ext}
            ${homepage} FILES ${copyright}
        )
    endif()

    list(TRANSFORM INFO_VCPKG_DEPENDENCIES REPLACE ":.*" "")
    foreach(dep IN LISTS INFO_VCPKG_DEPENDENCIES)
        ivw_vcpkg_install(${dep} MODULE ${ARG_MODULE} ${ext})
    endforeach()
    
    if(ARG_OUT_VERSION)
        set(${ARG_OUT_VERSION} ${INFO_VCPKG_VERSION} PARENT_SCOPE)
    endif()
    if(ARG_OUT_COPYRIGHT)
        set(${ARG_OUT_COPYRIGHT} ${copyright} PARENT_SCOPE)
    endif()

    #  HACK: have the files showing in the IDE
    if(NOT TARGET ${name}_vcpkg)
        add_custom_target(${name}_vcpkg SOURCES ${headers})
        source_group(
            TREE "${_VCPKG_ROOT_DIR}/installed/${VCPKG_TARGET_TRIPLET}/include/" 
            PREFIX "Header Files" 
            FILES ${headers}
        )
        set_target_properties(${name}_vcpkg PROPERTIES FOLDER vcpkg)
    endif()

endfunction()
