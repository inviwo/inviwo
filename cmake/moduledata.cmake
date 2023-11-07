#################################################################################
# 
# Inviwo - Interactive Visualization Workshop
# 
# Copyright (c) 2023 Inviwo Foundation
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

function(ivw_private_setup_module_data)
    set(options)
    set(oneValueArgs DIR BASE NAME GROUP VERSION)
    set(multiValueArgs "")
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    foreach(item IN LISTS oneValueArgs)
        if(NOT ARG_${item})
            message(FATAL_ERROR "ivw_private_setup_module_data: ${item} not specified")
        endif()
    endforeach()
    if(ARG_KEYWORDS_MISSING_VALUES) 
        message(FATAL_ERROR "ivw_private_setup_module_data: Missing values for keywords ${ARG_KEYWORDS_MISSING_VALUES}")
    endif()
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "ivw_private_setup_module_data: Unparsed arguments ${ARG_UNPARSED_ARGUMENTS}")
    endif()

    set(dir ${ARG_DIR})
    set(name ${ARG_NAME})
    set(module_path ${ARG_BASE})
    ivw_dir_to_mod_dep(mod ${dir})
    ivw_dir_to_mod_prefix(opt ${dir})           # OpenGL -> IVW_MODULE_OPENGL
    ivw_dir_to_module_taget_name(target ${dir}) # OpenGL -> inviwo-module-opengl

    ivw_private_get_ivw_module_include_path(${module_path}/${dir} includePrefix includePath orgName)
    # includePrefix -> inviwo/opengl
    # includePath -> ... inviwo/modules/opengl/include/inviwo/opengl
    # orgName -> inviwo

    set(class "${name}Module")
    set(alias "inviwo::module::${dir}")
    set(header "${includePrefix}/${dir}module.h")
    string(TOUPPER "${name}" u_module)
    set(api "IVW_MODULE_${u_module}_API")
    set(apiDefineInc ${includePrefix}/${dir}moduledefine.h)
    set(sharedLibInc ${includePrefix}/${dir}modulesharedlibrary.h)
    set(sharedLibCpp ${CMAKE_BINARY_DIR}/modules/${dir}/src/${dir}modulesharedlibrary.cpp)
    set(sharedLibHpp ${CMAKE_BINARY_DIR}/modules/${dir}/include/${sharedLibInc})

    # Get module version
    set("${mod}_name"         "${name}"               CACHE INTERNAL "Module name")
    set("${mod}_dir"          "${dir}"                CACHE INTERNAL "Module dir")
    set("${mod}_base"         "${module_path}"        CACHE INTERNAL "Module base")
    set("${mod}_path"         "${module_path}/${dir}" CACHE INTERNAL "Module path")
    set("${mod}_group"        "${ARG_GROUP}"          CACHE INTERNAL "Module group")
    set("${mod}_opt"          "${opt}"                CACHE INTERNAL "Module cmake option")
    set("${mod}_target"       "${target}"             CACHE INTERNAL "Module target")
    set("${mod}_alias"        "${alias}"              CACHE INTERNAL "Module alias")
    set("${mod}_class"        "${class}"              CACHE INTERNAL "Module class")
    set("${mod}_modName"      "Inviwo${name}Module"   CACHE INTERNAL "Module mod name")
    set("${mod}_version"      "${ARG_VERSION}"        CACHE INTERNAL "Module version")
    set("${mod}_incPath"      "${includePath}"        CACHE INTERNAL "Module include Path")
    set("${mod}_header"       "${header}"             PARENT_SCOPE) # Module header for file generation
    set("${mod}_api"          "${api}"                PARENT_SCOPE) # API Macro for file generation
    set("${mod}_apiDefineInc" "${apiDefineInc}"       PARENT_SCOPE) # API header include for file generation
    set("${mod}_sharedLibInc" "${sharedLibInc}"       PARENT_SCOPE) # Shared lib include for file generation
    set("${mod}_sharedLibCpp" "${sharedLibCpp}"       PARENT_SCOPE) # Shared lib Source for file generation
    set("${mod}_sharedLibHpp" "${sharedLibHpp}"       PARENT_SCOPE) # Shared lib Header for file generation

    # Check of there is a depends.cmake
    # Optionally defines: dependencies, aliases, protected, EnableByDefault
    # Save dependencies to INVIWO<NAME>MODULE_dependencies
    # Save aliases to INVIWO<NAME>MODULE_aliases
    # Save protected to INVIWO<NAME>MODULE_protected
    # Save EnableByDefault to INVIWO<NAME>MODULE_EnableByDefault
    set(dependencies "")
    set(aliases "")
    set(protected OFF)
    set(EnableByDefault OFF)
    set(Disabled OFF)
    set(DisabledReason "")
    if(EXISTS "${${mod}_path}/depends.cmake")
        include(${${mod}_path}/depends.cmake)
    endif()

    # set by ivw_enable_modules_if to enable non modules to force modules to build
    if(DEFINED ${mod}_enableExternal)
        set(EnableByDefault ${${mod}_enableExternal})
    endif()

    list(PREPEND dependencies InviwoCoreModule)
    set("${mod}_dependencies"    ${dependencies}    CACHE INTERNAL "Module dependencies")
    set("${mod}_protected"       ${protected}       PARENT_SCOPE)
    set("${mod}_enableByDefault" ${EnableByDefault} PARENT_SCOPE)
    set("${mod}_disabled"        ${Disabled}        PARENT_SCOPE)
    set("${mod}_disabledReason"  ${DisabledReason}  PARENT_SCOPE)
    set("${mod}_aliases" ${aliases} CACHE INTERNAL "Module aliases")

    # Check if there is a readme.md of the module.
    # In that case set to INVIWO<NAME>MODULE_description
    if(EXISTS "${${mod}_path}/readme.md")
        file(READ "${${mod}_path}/readme.md" description)
        # truncate description since some readme files are quite substantial
        string(LENGTH "${description}" desc_len)
        if(desc_len GREATER 250)
            string(SUBSTRING "${description}" 0 250 description)
            string(JOIN "" description "${description}" "...")
        endif()
        # encode linebreaks, i.e. '\n', and semicolon in description for
        # proper handling in CMAKE
        encodeLineBreaks(cdescription ${description})
        set("${mod}_description" ${cdescription} PARENT_SCOPE)
    endif()
endfunction()