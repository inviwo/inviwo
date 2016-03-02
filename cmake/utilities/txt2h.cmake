 #################################################################################
 #
 # Inviwo - Interactive Visualization Workshop
 #
 # Copyright (c) 2013-2015 Inviwo Foundation
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

#--------------------------------------------------------------------
# Create CMAKE file for pre-process 
function(ivw_text_to_header string_name retval)
    string(TOUPPER ${string_name} u_string_name)
    set(the_list "#ifndef ${u_string_name}\n")
    set(the_list "${the_list}#define ${u_string_name}\n\n")
    set(the_list "${the_list}#include <string>\n\n")
    set(the_list "${the_list}const std::string ${string_name} =\n")
    set(items "${ARGN}")
    string(REPLACE "\\" "\\\\" items ${items})
    string(REPLACE "\"" "\\\"" items ${items})
    string(REPLACE "\n" "\\n\"\n\"" items ${items})
    set(the_list "${the_list}\"${items}\";\n")
    set(the_list "${the_list}\n")
    set(the_list "${the_list}#endif\n")

    set(${retval} "${the_list}" PARENT_SCOPE)
endfunction()

function(ivw_generate_shader_header name glslfile headerfile)
    file(READ ${glslfile} glslcode)
    string(REPLACE ";" "__SEMICOLON__" glslcode "${glslcode}")
    ivw_text_to_header(${name} header ${glslcode})
    string(REPLACE "__SEMICOLON__" ";" header "${header}")
    file(WRITE ${headerfile} "${header}")
endfunction()
