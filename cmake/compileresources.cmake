#################################################################################
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2013-2020 Inviwo Foundation
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

option(IVW_CFG_EMBED_SHADERS 
    [[Embed shaders files as code in the executable. Makes distribution easier, but removes possibility of runtime modifications.]] 
    OFF
)

function(ivw_handle_shader_resources path)
    # Handle external resources
    if(IVW_CFG_EMBED_SHADERS)
        ivw_generate_shader_resource(${path} ${ARGN})
    else()
        # Add shader directory to pack
        ivw_add_to_module_pack(${path})

        string(TOUPPER ${PROJECT_NAME} u_project_name)
        set(U_MODULE ${u_project_name})
        string(TOLOWER ${PROJECT_NAME} l_project_name)
        set(L_MODULE ${l_project_name})
        set(ADD_INCLUDES "")
        set(ADD_RESOURCES "")
        ivw_private_get_ivw_module_include_path(${CMAKE_CURRENT_LIST_DIR} includePrefix includePath orgName)
        configure_file(${IVW_CMAKE_TEMPLATES}/shader_resource_template.h 
                       ${CMAKE_CURRENT_BINARY_DIR}/include/${includePrefix}/shader_resources.h)
    endif()
endfunction()


#--------------------------------------------------------------------
# Create CMAKE file for pre-process 
function(ivw_generate_shader_resource parent_path)
    set(cmd "include(${IVW_ROOT_DIR}/cmake/utilities/txt2h.cmake)\n")
    set(includes "")
    set(resources "")

    string(TOUPPER ${PROJECT_NAME} u_project_name)
    string(TOLOWER ${PROJECT_NAME} l_project_name)

    ivw_private_get_ivw_module_include_path(${CMAKE_CURRENT_LIST_DIR} includePrefix includePath orgName)

    foreach(shader_path ${ARGN})
        file(RELATIVE_PATH shaderkey ${parent_path} ${shader_path})
        string(REPLACE "/" "_" tmp ${shaderkey})
        string(REPLACE "." "_" tmp ${tmp})
        string(REPLACE " " "_" varName ${tmp})

        set(headerpath "${includePrefix}/glsl/${varName}.h")

        set(outfile "${CMAKE_CURRENT_BINARY_DIR}/include/${headerpath}")
        set(cmd "${cmd}ivw_generate_shader_header(\"${varName}\" \"${shader_path}\" \"${outfile}\")\n")
        set(includes "${includes}#include <${headerpath}>\n")
        set(resources "${resources}    manager->addShaderResource(\"${shaderkey}\", ${varName});\n")
    endforeach()

    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/create_shader_resource.cmake ${cmd})

    add_definitions(-D${u_project_name}_INCLUDE_SHADER_RESOURCES)

    set(U_MODULE ${u_project_name})
    set(L_MODULE ${l_project_name})
    set(ADD_INCLUDES "${includes}")
    set(ADD_RESOURCES "${resources}")

    configure_file(${IVW_CMAKE_TEMPLATES}/shader_resource_template.h 
                   ${CMAKE_CURRENT_BINARY_DIR}/include/${includePrefix}/shader_resources.h)

    add_custom_target("inviwo-shader-resources-${l_project_name}"
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/create_shader_resource.cmake
        DEPENDS ${ARGN}
        WORKING_DIRECTORY ${OUTPUT_DIR}
        COMMENT "Generating Shader resources for ${PROJECT_NAME}"
        VERBATIM
    )
    set_target_properties("inviwo-shader-resources-${l_project_name}" PROPERTIES FOLDER "shader-resources")
    add_dependencies(inviwo-module-${l_project_name} inviwo-shader-resources-${l_project_name})

endfunction()