#################################################################################
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2020-2023 Inviwo Foundation
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

# Retrive the git sha of the VCPKG installation, once for later use in ivw_vcpkg_install
if(VCPKG_TOOLCHAIN)
    ivw_git_get_hash(${Z_VCPKG_ROOT_DIR} ivw_vcpkg_sha)
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

    if(NOT Python3_Interpreter_FOUND)
        return()
    endif()

    set(options EXT)
    set(oneValueArgs OUT_COPYRIGHT OUT_VERSION MODULE)
    set(multiValueArgs "")
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	string(TOLOWER "${name}" lowercase_name)

    set(install "--install" "${VCPKG_INSTALLED_DIR}")
    set(installdir "${VCPKG_INSTALLED_DIR}/")

    if(NOT DEFINED ivw_vcpkg_info_${lowercase_name} OR 
        NOT ivw_vcpkg_info_${lowercase_name}_sha STREQUAL ivw_vcpkg_sha)
        message(STATUS "Vcpkg fetching metadata for: ${name}")
        execute_process(
            COMMAND "${Python3_EXECUTABLE}" "${IVW_TOOLS_DIR}/vcpkginfo.py"
                --vcpkg "${Z_VCPKG_EXECUTABLE}" 
                --pkg ${lowercase_name}
                --triplet ${VCPKG_TARGET_TRIPLET}
                --manifest_dir ${VCPKG_MANIFEST_DIR}
                ${install}
            RESULT_VARIABLE returnCode
            OUTPUT_VARIABLE pkgInfo
            ERROR_VARIABLE pkgError
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(NOT returnCode EQUAL "0")
            message(WARNING "  Unable to retrive vcpkg package info for ${name}.\n" 
                "  vcpkg: ${Z_VCPKG_EXECUTABLE}\n"
                "  triplet: ${VCPKG_TARGET_TRIPLET}\n"
                "  package: ${lowercase_name}\n"
                "  manifest ${VCPKG_MANIFEST_DIR}\n"
                "  install: ${install}\n"
                "  return:  ${returnCode}\n"
                "  stdout:  ${pkgInfo}\n"  
                "  stderr:  ${pkgError}\n"
            )
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

    if(NOT INFO_VCPKG_OWNED_FILES)
        set(INFO_VCPKG_OWNED_FILES "")
    endif()

    if(WIN32)
        set(binfiles ${INFO_VCPKG_OWNED_FILES})
        list(FILTER binfiles INCLUDE REGEX "${VCPKG_TARGET_TRIPLET}/bin/.*\\.dll") 
        list(TRANSFORM binfiles PREPEND ${installdir})

        set(pdbfiles ${INFO_VCPKG_OWNED_FILES})
        list(FILTER pdbfiles INCLUDE REGEX "${VCPKG_TARGET_TRIPLET}/bin/.*\\.pdb")
        list(TRANSFORM pdbfiles PREPEND ${installdir})

        set(libfiles ${INFO_VCPKG_OWNED_FILES})
        list(FILTER libfiles INCLUDE REGEX "${VCPKG_TARGET_TRIPLET}/lib/.*\\.lib")
        list(TRANSFORM libfiles PREPEND ${installdir})

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
    elseif(APPLE)
        set(libfiles ${INFO_VCPKG_OWNED_FILES})
        list(FILTER libfiles INCLUDE REGEX "${VCPKG_TARGET_TRIPLET}/lib/.*\\.dylib")
        list(TRANSFORM libfiles PREPEND ${installdir})
        install(
            FILES ${libfiles} 
            DESTINATION ${IVW_LIBRARY_INSTALL_DIR}
            COMPONENT Application
        )
        
        set(libfiles ${INFO_VCPKG_OWNED_FILES})
        list(FILTER libfiles INCLUDE REGEX "${VCPKG_TARGET_TRIPLET}/lib/.*\\.a")
        list(TRANSFORM libfiles PREPEND ${installdir})
        install(
            FILES ${libfiles} 
            DESTINATION ${IVW_LIBRARY_INSTALL_DIR}
            COMPONENT Development
        )
    else()
        set(libfiles ${INFO_VCPKG_OWNED_FILES})
        list(FILTER libfiles INCLUDE REGEX "${VCPKG_TARGET_TRIPLET}/lib/.*\\.so")
        list(TRANSFORM libfiles PREPEND ${installdir})
        install(
            FILES ${libfiles} 
            DESTINATION ${IVW_LIBRARY_INSTALL_DIR}
            COMPONENT Application
        )
        
        set(libfiles ${INFO_VCPKG_OWNED_FILES})
        list(FILTER libfiles INCLUDE REGEX "${VCPKG_TARGET_TRIPLET}/lib/.*\\.a")
        list(TRANSFORM libfiles PREPEND ${installdir})
        install(
            FILES ${libfiles} 
            DESTINATION ${IVW_LIBRARY_INSTALL_DIR}
            COMPONENT Development
        )
    endif()

    set(copyright ${INFO_VCPKG_OWNED_FILES})
    list(FILTER copyright INCLUDE REGEX "${VCPKG_TARGET_TRIPLET}/share/.*copyright.*")
    list(TRANSFORM copyright PREPEND ${installdir})

    set(headers ${INFO_VCPKG_OWNED_FILES})
    list(FILTER headers INCLUDE REGEX "${VCPKG_TARGET_TRIPLET}/include/.*\\..?.*")
    list(TRANSFORM headers PREPEND ${installdir})


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

    if(INFO_VCPKG_DEPENDENCIES)
        list(TRANSFORM INFO_VCPKG_DEPENDENCIES REPLACE ":.*" "")
        foreach(dep IN LISTS INFO_VCPKG_DEPENDENCIES)
            if(NOT dep MATCHES "^vcpkg-")
                ivw_vcpkg_install(${dep} MODULE ${ARG_MODULE} ${ext})
            endif()
        endforeach()
    endif()
    
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
            TREE "${installdir}${VCPKG_TARGET_TRIPLET}/include/" 
            PREFIX "Header Files" 
            FILES ${headers}
        )
        set_target_properties(${name}_vcpkg PROPERTIES FOLDER vcpkg)
    endif()

endfunction()
