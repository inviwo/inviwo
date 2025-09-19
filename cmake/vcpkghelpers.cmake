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

# Retrieve the git SHA of the VCPKG installation, once for later use in ivw_vcpkg_install
if(VCPKG_TOOLCHAIN)
    ivw_git_get_hash(${Z_VCPKG_ROOT_DIR} ivw_vcpkg_sha)
endif()

function(ivw_private_vcpkg_install_helper)
    set(options "")
    set(oneValueArgs FILES_VAR EXTENSION DIRNAME DESTINATION COMPONENT)
    set(multiValueArgs )
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs};FOUND_VAR" "${multiValueArgs}" ${ARGN})

    foreach(item IN LISTS oneValueArgs)
        if(NOT ARG_${item})
            message(FATAL_ERROR "ivw_private_vcpkg_install_helper: ${item} not set")
        endif()
    endforeach()
    if(ARG_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR "ivw_private_vcpkg_install_helper: Missing values for keywords ${ARG_KEYWORDS_MISSING_VALUES}")
    endif()
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "ivw_private_vcpkg_install_helper: Unparsed arguments ${ARG_UNPARSED_ARGUMENTS}")
    endif()

    if(ARG_CONFIGURATIONS)
        set(configurations CONFIGURATIONS ${ARG_CONFIGURATIONS})
    else()
        set(configurations )
    endif()

    set(matched_files "")

    set(files ${${ARG_FILES_VAR}})
    list(FILTER files INCLUDE REGEX "^${VCPKG_TARGET_TRIPLET}/${ARG_DIRNAME}/[^/]+\\.${ARG_EXTENSION}")
    list(APPEND matched_files ${files})
    list(TRANSFORM files PREPEND ${VCPKG_INSTALLED_DIR}/)
    install(
        FILES ${files}
        DESTINATION ${ARG_DESTINATION}
        COMPONENT ${ARG_COMPONENT}
        CONFIGURATIONS Release RelWithDebInfo MinSizeRel
    )

    set(files ${${ARG_FILES_VAR}})
    list(FILTER files INCLUDE REGEX "^${VCPKG_TARGET_TRIPLET}/${ARG_DIRNAME}/[^/]+/[^/]+\\.${ARG_EXTENSION}")
    list(APPEND matched_files ${files})
    foreach(item IN LISTS files)
        cmake_path(GET item PARENT_PATH parentpath)
        cmake_path(GET parentpath FILENAME parentname)
        install(
            FILES ${VCPKG_INSTALLED_DIR}/${item}
            DESTINATION ${ARG_DESTINATION}/${parentname}
            COMPONENT ${ARG_COMPONENT}
            CONFIGURATIONS Release RelWithDebInfo MinSizeRel
        )
    endforeach()

    set(files ${${ARG_FILES_VAR}})
    list(FILTER files INCLUDE REGEX "^${VCPKG_TARGET_TRIPLET}/debug/${ARG_DIRNAME}/[^/]+\\.${ARG_EXTENSION}")
    list(APPEND matched_files ${files})
    list(TRANSFORM files PREPEND ${VCPKG_INSTALLED_DIR}/)
    install(
        FILES ${files}
        DESTINATION ${ARG_DESTINATION}
        COMPONENT ${ARG_COMPONENT}
        CONFIGURATIONS Debug
    )

    set(files ${${ARG_FILES_VAR}})
    list(FILTER files INCLUDE REGEX "^${VCPKG_TARGET_TRIPLET}/debug/${ARG_DIRNAME}/[^/]+/[^/]+\\.${ARG_EXTENSION}")
    list(APPEND matched_files ${files})
    foreach(item IN LISTS files)
        cmake_path(GET item PARENT_PATH parentpath)
        cmake_path(GET parentpath FILENAME parentname)
        install(
            FILES ${VCPKG_INSTALLED_DIR}/${item}
            DESTINATION ${ARG_DESTINATION}/${parentname}
            COMPONENT ${ARG_COMPONENT}
            CONFIGURATIONS Debug
        )
    endforeach()

    if(ARG_FOUND_VAR)
        set(${ARG_FOUND_VAR} ${matched_files} PARENT_SCOPE)
    endif()
endfunction()

# Reset global variable used to show Python warning in ivw_vcpkg_install only once
unset(ivwVcpkgInstallPythonWarningShownOnce CACHE)


set(IVW_VCPKG_DUMMY_TARGETS_INCLUDE ".*" CACHE STRING "List of CMake REGEXs for targets to make dummy targets for")
set(IVW_VCPKG_DUMMY_TARGETS_EXCLUDE "" CACHE STRING "List of CMake REGEXs for targets to not make dummy targets for")


# A helper function to install vcpkg libraries. Will install dll/so, lib, pdb, etc. into the 
# corresponding folders by globbing the vcpkg package folders. 
# It will also try and install any transitive dependencies automatically 
# We also automatically register the license file using the metadata found in vcpkg
# Requires a Python3 interpreter.
# Parameters:
#  * OUT_COPYRIGHT retrieve the path the the COPYRIGHT file
#  * OUT_VERSION get the package version from the vcpkg metadata
#  * MODULE the module to use for ivw_register_license_file
#  * EXT pass on to ivw_register_license_file
function(ivw_vcpkg_install name)
    if(NOT VCPKG_TOOLCHAIN)
        return()
    endif()

    set(options EXT)
    set(oneValueArgs OUT_COPYRIGHT OUT_VERSION MODULE EXTRA_CODE)
    set(multiValueArgs "")
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    string(TOLOWER "${name}" lowercase_name)

    # use a global property (IVW_VCPKG_INSTALLED_PACKAGES) to avoid installing
    # the same package multiple times. This is comman with shared dependencies.
    get_property(visited GLOBAL PROPERTY IVW_VCPKG_INSTALLED_PACKAGES)
    if(NOT visited)
        set(visited "")
    endif()
    if(lowercase_name IN_LIST visited)
        return()
    endif()
    list(APPEND visited ${lowercase_name})
    set_property(GLOBAL PROPERTY IVW_VCPKG_INSTALLED_PACKAGES ${visited})


    if(NOT Python3_Interpreter_FOUND)
        find_package(Python3 COMPONENTS Interpreter)
        if(NOT Python3_Interpreter_FOUND)
            if(NOT ivwVcpkgInstallPythonWarningShownOnce)
                message(WARNING "Python3 not available.\n"
                    "Python is required to install vcpkg libraries. Please install Python3. If you "
                    "have a Python installation that is not found, consider "
                    "setting Python3_ROOT_DIR to the root directory of your Python 3 installation."
                )
                set(ivwVcpkgInstallPythonWarningShownOnce ON CACHE INTERNAL "vcpkg_install Python warning shown once")
            endif()
            return ()
        endif()
    endif()

    set(install "--install" "${VCPKG_INSTALLED_DIR}")
    set(installdir "${VCPKG_INSTALLED_DIR}/")

    if(NOT DEFINED ivw_vcpkg_info_${lowercase_name} OR 
        NOT ivw_vcpkg_info_${lowercase_name}_sha STREQUAL ivw_vcpkg_sha)
        message(STATUS "Vcpkg fetching metadata for: ${name}")
        execute_process(
            COMMAND "${Python3_EXECUTABLE}" "${IVW_ROOT_DIR}/tools/vcpkginfo.py"
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

    set(vcOptions "")
    set(vcOneValueArgs VCPKG_VERSION VCPKG_HOMEPAGE VCPKG_DESCRIPTION)
    set(vcMultiValueArgs VCPKG_DEPENDENCIES VCPKG_OWNED_FILES)
    cmake_parse_arguments(INFO "${vcOptions}" "${vcOneValueArgs}" "${vcMultiValueArgs}" ${pkgInfo})

    if(NOT INFO_VCPKG_OWNED_FILES)
        set(INFO_VCPKG_OWNED_FILES "")
    endif()

    set(includes ${INFO_VCPKG_OWNED_FILES})
    list(FILTER includes INCLUDE REGEX "^${VCPKG_TARGET_TRIPLET}/include/")

    set(basedirs ${includes})
    list(FILTER basedirs INCLUDE REGEX "^${VCPKG_TARGET_TRIPLET}/include/[^/]+/$")
    list(TRANSFORM basedirs REPLACE "/$" "")
    list(TRANSFORM basedirs PREPEND ${installdir})
    
    set(baseheaders ${includes})
    list(FILTER baseheaders INCLUDE REGEX "^${VCPKG_TARGET_TRIPLET}/include/[^/]+$")
    list(TRANSFORM baseheaders PREPEND ${installdir})
    
    set(headers ${includes})
    list(FILTER headers INCLUDE REGEX "^${VCPKG_TARGET_TRIPLET}/include/.+[^/]$")
    list(TRANSFORM headers PREPEND ${installdir})

    install(
        FILES ${baseheaders} 
        DESTINATION ${IVW_INCLUDE_INSTALL_DIR}
        COMPONENT Development
    )
    install(
        DIRECTORY ${basedirs} 
        DESTINATION ${IVW_INCLUDE_INSTALL_DIR}
        COMPONENT Development
    )

    set(config ${INFO_VCPKG_OWNED_FILES})
    list(FILTER config INCLUDE REGEX "${VCPKG_TARGET_TRIPLET}/share/[^/]+/$")
    list(TRANSFORM config REPLACE "/$" "")
    list(TRANSFORM config PREPEND ${installdir})
    install(
        DIRECTORY ${config} 
        DESTINATION ${IVW_SHARE_INSTALL_DIR}
        COMPONENT Development
        FILES_MATCHING PATTERN "*.cmake"
    )

    if(WIN32)
        ivw_private_vcpkg_install_helper(
            FILES_VAR INFO_VCPKG_OWNED_FILES
            DIRNAME "bin"
            EXTENSION "(dll|pdb)"
            DESTINATION ${IVW_RUNTIME_INSTALL_DIR}
            COMPONENT Application
        )
        ivw_private_vcpkg_install_helper(
            FILES_VAR INFO_VCPKG_OWNED_FILES
            DIRNAME "lib"
            EXTENSION "lib"
            DESTINATION ${IVW_ARCHIVE_INSTALL_DIR}
            COMPONENT Development
        )
    elseif(APPLE)
        ivw_private_vcpkg_install_helper(
            FILES_VAR INFO_VCPKG_OWNED_FILES
            DIRNAME "lib"
            EXTENSION "(dylib|a)"
            DESTINATION ${IVW_LIBRARY_INSTALL_DIR}
            COMPONENT Application
        )
    else()
        ivw_private_vcpkg_install_helper(
            FILES_VAR INFO_VCPKG_OWNED_FILES
            DIRNAME "lib"
            EXTENSION "(so|a)"
            DESTINATION ${IVW_LIBRARY_INSTALL_DIR}
            COMPONENT Application
        )
    endif()

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

    set(copyright ${INFO_VCPKG_OWNED_FILES})
    list(FILTER copyright INCLUDE REGEX "${VCPKG_TARGET_TRIPLET}/share/.*copyright.*")
    list(TRANSFORM copyright PREPEND ${installdir})
    if(copyright)
        ivw_register_license_file(NAME ${name} 
            VERSION ${INFO_VCPKG_VERSION} MODULE ${ARG_MODULE} ${ext}
            ${homepage} FILES ${copyright}
        )
    endif()

    if(ARG_EXTRA_CODE)
        cmake_language(EVAL CODE ${ARG_EXTRA_CODE})
    endif()

    if(INFO_VCPKG_DEPENDENCIES)
        list(TRANSFORM INFO_VCPKG_DEPENDENCIES REPLACE ":.*" "")
        list(REMOVE_DUPLICATES INFO_VCPKG_DEPENDENCIES)
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
        set(includeit FALSE)
        foreach(pattern IN LISTS IVW_VCPKG_DUMMY_TARGETS_INCLUDE)
            string(REGEX MATCH "${pattern}" match ${name})
            if(match)
                set(includeit TRUE)
                break()
            endif()
        endforeach()
        foreach(pattern IN LISTS IVW_VCPKG_DUMMY_TARGETS_EXCLUDE)
            string(REGEX MATCH "${pattern}" match ${name})
            if(match)
                set(includeit FALSE)
                break()
            endif()
        endforeach()

        if(includeit)
            message(STATUS "Creating dummy target for vcpkg package: ${name}")
            add_custom_target(${name}_vcpkg SOURCES ${headers})
            source_group(
                TREE "${installdir}${VCPKG_TARGET_TRIPLET}/include/"
                PREFIX "Header Files"
                FILES ${headers}
            )
            set_target_properties(${name}_vcpkg PROPERTIES FOLDER vcpkg)
        endif()
    endif()

    ivw_append_install_list(GLOBAL)

endfunction()
