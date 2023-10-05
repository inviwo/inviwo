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

include(CMakeParseArguments)

define_property(
    DIRECTORY PROPERTY IVW_LICENSE_LIST
    BRIEF_DOCS "List of license ids for this target"
    FULL_DOCS "List of license ids for this target"
)
define_property(
    TARGET PROPERTY IVW_LICENSE_LIST INHERITED
    BRIEF_DOCS "List of license ids for this target"
    FULL_DOCS "List of license ids for this target"
)

#--------------------------------------------------------------------
# Register a license file with inviwo. Will make the information 
# available in the module factory object. All the registed files can 
# be found in the About dialog in the Inviwo application. 
# Registed files will automatically be include in the installers.
# This function should be called from a module CMakeLists.txt file
# Parameters:
# * NAME (mandatory):   Name of the library to which the license belong.
# * MODULE (mandatory): Inviwo Module to which the library is associated. 
# * TARGET (optional):  Library target. Used to get library VERSION using  
#                       get_target_property(ver target VERSION) if VERSION 
#                       argument is not specified.
# * ID (optional):      An internal id used to identify the license, should
#                       only be lowercase letters and numbers. If not given
#                       the name will be cleaned and used.
# * VERSION (optional): Version of the library to which the license belong.
# * URL (optional):     Url of the library to which the license belong.
# * FILES (optional):   List of license file for library
# * TYPE (optional):    The type of license
#
# Example usage: 
#    ivw_register_license_file(NAME "GLFW" VERSION 3.1.0 MODULE GLFW
#        URL http://www.glfw.org
#        TYPE "The zlib/libpng License"
#        FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/COPYING.txt
#    )
function(ivw_register_license_file)
    set(options EXT)
    set(oneValueArgs TARGET ID NAME VERSION URL MODULE TYPE)
    set(multiValueArgs FILES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    if(NOT ARG_NAME)
        message(FATAL_ERROR "NAME not specified in call to ivw_register_license_file")
    endif()
    if(NOT ARG_MODULE)
        message(FATAL_ERROR "MODULE not specified in call to ivw_register_license_file")
    endif()
    if(NOT ARG_ID)
        string(TOLOWER ${ARG_NAME} id)
        string(REGEX REPLACE [=[[^a-z0-9]]=] "" id ${id})
        set(ARG_ID ${id})
    endif()
    if(NOT ARG_TYPE)
        set(ARG_TYPE "License")
    endif()
    if(ARG_TARGET AND NOT ARG_VERSION)
        get_target_property(ARG_VERSION ${ARG_TARGET} VERSION)
    endif()
    if(NOT ARG_VERSION)
        set(ARG_VERSION "0.0.0")
    endif()

    if(ARG_EXT)
        set(scope DIRECTORY)
    else()
        ivw_dir_to_mod_dep(mod ${ARG_MODULE})
        if(NOT TARGET ${${mod}_target})
            message(FATAL_ERROR "ivw_register_license_file should be called from a module CMakeLists.txt")
        endif()
        set(scope TARGET ${${mod}_target})
    endif()

    get_property(license_list ${scope} PROPERTY IVW_LICENSE_LIST)
    if(NOT license_list) 
        set(license_list "")
    endif()
    list(APPEND license_list ${ARG_ID})
    set_property(${scope} PROPERTY IVW_LICENSE_LIST ${license_list})
    string(TOLOWER ${ARG_MODULE} lmodule)
    if(${ARG_MODULE} STREQUAL "Core")
        set(installDest ${IVW_RESOURCE_INSTALL_PREFIX}licenses/)
    else()
        set(installDest ${IVW_RESOURCE_INSTALL_PREFIX}modules/${lmodule}/licenses/)
    endif()

    set(files "")
    set(fileCount 0)
    foreach(file IN LISTS ARG_FILES)
        get_filename_component(filename ${file} NAME)
        set(dest "${IVW_BINARY_DIR}/modules/${lmodule}/licenses/${ARG_ID}-${filename}-${fileCount}.txt")
        configure_file(${file} ${dest} COPYONLY)
        install(FILES ${dest} DESTINATION ${installDest} COMPONENT Application)
        list(APPEND files ${ARG_ID}-${filename}-${fileCount}.txt)
        matH(EXPR fileCount "${fileCount} + 1")
    endforeach()
    
    set_property(GLOBAL PROPERTY "ivw_license_${ARG_ID}_name"    "${ARG_NAME}")
    set_property(GLOBAL PROPERTY "ivw_license_${ARG_ID}_version" "${ARG_VERSION}")
    set_property(GLOBAL PROPERTY "ivw_license_${ARG_ID}_url"     "${ARG_URL}")
    set_property(GLOBAL PROPERTY "ivw_license_${ARG_ID}_files"   "${files}")
    set_property(GLOBAL PROPERTY "ivw_license_${ARG_ID}_module"  "${ARG_MODULE}")
    set_property(GLOBAL PROPERTY "ivw_license_${ARG_ID}_type"    "${ARG_TYPE}")
endfunction()


#--------------------------------------------------------------------
# Generate license information for the module registration header files
# this function is called by ivw_private_generate_module_registration_file
function(ivw_private_generate_license_header)
    set(options "")
    set(oneValueArgs MOD RETVAL)
    set(multiValueArgs "")
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    get_property(license_list TARGET ${${ARG_MOD}_target} PROPERTY IVW_LICENSE_LIST)
    set(licenses "")
    foreach(id IN LISTS license_list)
        get_property(name    GLOBAL PROPERTY "ivw_license_${id}_name")
        get_property(version GLOBAL PROPERTY "ivw_license_${id}_version")
        get_property(url     GLOBAL PROPERTY "ivw_license_${id}_url")
        get_property(files   GLOBAL PROPERTY "ivw_license_${id}_files")
        get_property(module  GLOBAL PROPERTY "ivw_license_${id}_module")
        get_property(type    GLOBAL PROPERTY "ivw_license_${id}_type")

        # LicenseInfo(const std::string& id, const std::string& name, const Version& version,
        #             const std::string& module, const std::vector<std::string>& files);
        list_to_stringvector(files ${files})
        set(license "{\"${id}\", // License id"
                     "\"${name}\", // Name"
                     "\"${version}\", // Version"
                     "\"${url}\", // URL"
                     "\"${module}\", // Module"
                     "\"${type}\", // Type"
                     "${files}}")
        ivw_join(";" "\n          " license ${license})
        list(APPEND licenses "${license}")
    endforeach(id)
    ivw_join(";" ",\n         " licenses ${licenses})
    set(${ARG_RETVAL} "{${licenses}}" PARENT_SCOPE)
endfunction()
