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


 ####  Memory leak checks ####

IF(WIN32)
    IF(MSVC)
        if(IVW_ENABLE_MSVC_MEMLEAK_TEST)
            add_definitions(-DIVW_ENABLE_MSVC_MEM_LEAK_TEST)
            if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
                link_directories(${IVW_EXTENSIONS_DIR}/vld/lib/Win64)
            else ()
                link_directories(${IVW_EXTENSIONS_DIR}/vld/lib/Win32)
            endif()
        endif(IVW_ENABLE_MSVC_MEMLEAK_TEST)    
    ENDIF(MSVC)
ENDIF(WIN32)

function(ivw_memleak_setup unittest_target)
	if(IVW_ENABLE_MSVC_MEMLEAK_TEST)
	    IF(WIN32)
	        IF(MSVC)
	            if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	                add_custom_command(TARGET ${unittest_target} POST_BUILD 
	                                    COMMAND ${CMAKE_COMMAND}  -E copy_if_different 
	                                        ${IVW_EXTENSIONS_DIR}/vld/bin/Win64/vld_x64.dll 
	                                        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/vld_x64.dll)    
	                                        
	                add_custom_command(TARGET ${unittest_target} POST_BUILD 
	                                    COMMAND ${CMAKE_COMMAND}  -E copy_if_different 
	                                        ${IVW_EXTENSIONS_DIR}/vld/bin/Win64/dbghelp.dll 
	                                        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/dbghelp.dll)    
	                                        
	                add_custom_command(TARGET ${unittest_target} POST_BUILD 
	                                    COMMAND ${CMAKE_COMMAND}  -E copy_if_different 
	                                        ${IVW_EXTENSIONS_DIR}/vld/bin/Win64/Microsoft.DTfW.DHL.manifest 
	                                        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/Microsoft.DTfW.DHL.manifest)    
	            else ()
	                add_custom_command(TARGET ${unittest_target} POST_BUILD 
	                                    COMMAND ${CMAKE_COMMAND}  -E copy_if_different 
	                                        ${IVW_EXTENSIONS_DIR}/vld/bin/Win32/vld_x86.dll 
	                                        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/vld_x86.dll)    
	                                        
	                add_custom_command(TARGET ${unittest_target} POST_BUILD 
	                                    COMMAND ${CMAKE_COMMAND}  -E copy_if_different 
	                                        ${IVW_EXTENSIONS_DIR}/vld/bin/Win32/dbghelp.dll 
	                                        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/dbghelp.dll)    
	                                        
	                add_custom_command(TARGET ${unittest_target} POST_BUILD 
	                                    COMMAND ${CMAKE_COMMAND}  -E copy_if_different 
	                                        ${IVW_EXTENSIONS_DIR}/vld/bin/Win32/Microsoft.DTfW.DHL.manifest 
	                                        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/Microsoft.DTfW.DHL.manifest)    
	            endif()
	        ENDIF(MSVC)
	    ENDIF(WIN32)
	endif()
endfunction()