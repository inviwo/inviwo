#################################################################################
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2013-2019 Inviwo Foundation
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
    set(options "")
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

    ivw_dir_to_mod_dep(mod ${ARG_MODULE})
    if(NOT ${mod}_name)
         message(FATAL_ERROR "ivw_register_license_file should be called from a module CMakeLists.txt")
    endif()

    set("${mod}_licenses" "${${mod}_licenses};${ARG_ID}" CACHE INTERNAL "License ids")

    set(files "")
    foreach(file ${ARG_FILES})
        if(IS_ABSOLUTE ${file})
            if(${ARG_MODULE} STREQUAL "Core")
                file(RELATIVE_PATH relfile ${IVW_ROOT_DIR} ${file})
            else()
                file(RELATIVE_PATH relfile ${CMAKE_CURRENT_SOURCE_DIR} ${file})
            endif()
            list(APPEND files ${relfile})
        else()
            list(APPEND files ${file})
        endif()
    endforeach()
    
    set("ivw_licence_${ARG_ID}_name"    "${ARG_NAME}"    CACHE INTERNAL "License name")
    set("ivw_licence_${ARG_ID}_version" "${ARG_VERSION}" CACHE INTERNAL "License version")
    set("ivw_licence_${ARG_ID}_url"     "${ARG_URL}"     CACHE INTERNAL "License url")
    set("ivw_licence_${ARG_ID}_files"   "${files}"       CACHE INTERNAL "License files")
    set("ivw_licence_${ARG_ID}_module"  "${ARG_MODULE}"  CACHE INTERNAL "License module")
    set("ivw_licence_${ARG_ID}_type"    "${ARG_TYPE}"    CACHE INTERNAL "License type")

    if(${ARG_MODULE} STREQUAL "Core")
        if(APPLE)
            set(dest Inviwo.app/Contents/Resources/licenses/)
        else()
            set(dest licenses/)
        endif()
    else()
        if(APPLE)
            set(dest Inviwo.app/Contents/Resources/modules/${${mod}_dir}/licenses/)
        else()
            set(dest modules/${${mod}_dir}/licenses/)
        endif()
    endif()
    foreach(file ${ARG_FILES})
        get_filename_component(filename ${file} NAME)
        install(FILES ${file} DESTINATION ${dest} RENAME "${ARG_ID}-${filename}")
    endforeach()

endfunction()


#--------------------------------------------------------------------
# Generate license information for the module registration header files
# this function is called by ivw_private_generate_module_registration_file
function(ivw_private_generate_licence_header)
    set(options "")
    set(oneValueArgs MOD RETVAL)
    set(multiValueArgs "")
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(licenses "")
    foreach(id ${${ARG_MOD}_licenses})
        # LicenseInfo(const std::string& id, const std::string& name, const Version& version,
        #             const std::string& module, const std::vector<std::string>& files);
        list_to_stringvector(files ${ivw_licence_${id}_files})
        set(license "{\"${id}\", // License id"
                     "\"${ivw_licence_${id}_name}\", // Name"
                     "\"${ivw_licence_${id}_version}\", // Version"
                     "\"${ivw_licence_${id}_url}\", // URL"
                     "\"${ivw_licence_${id}_module}\", // Module"
                     "\"${ivw_licence_${id}_type}\", // Type"
                     "${files}}")
        ivw_join(";" "\n          " license ${license})
        list(APPEND licenses "${license}")
    endforeach(id)
    ivw_join(";" ",\n         " licenses ${licenses})
    set(${ARG_RETVAL} "{${licenses}}" PARENT_SCOPE)
endfunction()