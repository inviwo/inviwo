#################################################################################
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2013-2017 Inviwo Foundation
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
# Example usage: 
# ivw_register_license_file(ID glew NAME "GLEW" VERSION 2.0.0 MODULE OpenGL
#     URL http://glew.sourceforge.net
#     FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/glew/license.txt
# )
function(ivw_register_license_file)
    set(options "")
    set(oneValueArgs ID NAME VERSION URL MODULE)
    set(multiValueArgs FILES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    ivw_dir_to_mod_dep(mod ${ARG_MODULE})
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

    if(IVW_PACKAGE_PROJECT)
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
    endif()

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
                     "${files}}")
        join(";" "\n          " license ${license})
        list(APPEND licenses "${license}")
    endforeach(id)
    join(";" ",\n         " licenses ${licenses})
    set(${ARG_RETVAL} "{${licenses}}" PARENT_SCOPE)
endfunction()