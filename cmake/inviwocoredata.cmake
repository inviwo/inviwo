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

function(ivw_private_setup_core_data)
    set(dir "core")
    ivw_dir_to_mod_dep(mod ${dir}) # core -> IVW_CORE_MODULE
    ivw_dir_to_mod_prefix(opt ${dir}) # core -> IVW_MODULE_CORE

    set(module_path ${IVW_SOURCE_DIR})
    set(name "Core")
    set(class "InviwoCore")
    set(alias "inviwo::core")
    set(target "inviwo-core")
    set(header "inviwo/core/common/inviwocore.h")
    set(api "IVW_CORE_API")
    set(includePrefix "inviwo/core")
    set(includePath "${module_path}/${dir}/inviwo/core")
    set(apiDefineInc "inviwo/core/common/inviwocoredefine.h")
    set(sharedLibInc "inviwo/core/common/coremodulesharedlibrary.h")
    set(sharedLibCpp ${CMAKE_BINARY_DIR}/modules/${dir}/src/common/${dir}modulesharedlibrary.cpp)
    set(sharedLibHpp ${CMAKE_BINARY_DIR}/modules/${dir}/include/${sharedLibInc})

    set("${mod}_name"         "${name}"               CACHE INTERNAL "Module name")
    set("${mod}_dir"          "${dir}"                CACHE INTERNAL "Module dir")
    set("${mod}_base"         "${module_path}"        CACHE INTERNAL "Module base")
    set("${mod}_path"         "${module_path}/${dir}" CACHE INTERNAL "Module path")
    set("${mod}_group"        "core"                  CACHE INTERNAL "Module group")
    set("${mod}_opt"          "${opt}"                CACHE INTERNAL "Module cmake option")
    set("${mod}_target"       "${target}"             CACHE INTERNAL "Module target")
    set("${mod}_alias"        "${alias}"              CACHE INTERNAL "Module alias")
    set("${mod}_class"        "${class}"              CACHE INTERNAL "Module class")
    set("${mod}_modName"      "Inviwo${name}Module"   CACHE INTERNAL "Module mod name")
    set("${mod}_version"      "1.0.0"                 CACHE INTERNAL "Module version")
    set("${mod}_incPath"      "${includePath}"        CACHE INTERNAL "Module include Path")
    set("${mod}_header"       "${header}"             PARENT_SCOPE) # Module header for file generation
    set("${mod}_api"          "${api}"                PARENT_SCOPE) # API Macro for file generation
    set("${mod}_apiDefineInc" "${apiDefineInc}"       PARENT_SCOPE) # API header include for file generation
    set("${mod}_sharedLibInc" "${sharedLibInc}"       PARENT_SCOPE) # Shared lib include for file generation
    set("${mod}_sharedLibCpp" "${sharedLibCpp}"       PARENT_SCOPE) # Shared lib Source for file generation
    set("${mod}_sharedLibHpp" "${sharedLibHpp}"       PARENT_SCOPE) # Shared lib Header for file generation

    set("${mod}_dependencies"    ""   CACHE INTERNAL "Module dependencies")
    set("${mod}_protected"       ON   PARENT_SCOPE)
    set("${mod}_enableByDefault" ON   PARENT_SCOPE)
    set("${mod}_disabled"        OFF  PARENT_SCOPE)
    set("${mod}_disabledReason"  ""   PARENT_SCOPE)
    set("${mod}_aliases"         "" CACHE INTERNAL "Module aliases")
endfunction()
