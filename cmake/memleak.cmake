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

IF(WIN32 AND MSVC)
   	option(IVW_ENABLE_MSVC_MEMLEAK_TEST "Run memoryleak test within Visual Studio" OFF)
    if(IVW_ENABLE_MSVC_MEMLEAK_TEST)
        add_definitions(-DIVW_ENABLE_MSVC_MEM_LEAK_TEST)
        if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
            link_directories(${IVW_EXTENSIONS_DIR}/vld/lib/Win64)
        else ()
    	    link_directories(${IVW_EXTENSIONS_DIR}/vld/lib/Win32)
        endif()

        if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
            set(arch "Win64")
        else()
            set(arch "Win32")
        endif()

        add_custom_target("copy-memleak-lib"
           COMMAND ${CMAKE_COMMAND}  -E copy_if_different 
                ${IVW_EXTENSIONS_DIR}/vld/bin/${arch}/vld_x64.dll
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/vld_x64.dll 
            
            COMMAND ${CMAKE_COMMAND}  -E copy_if_different 
                ${IVW_EXTENSIONS_DIR}/vld/bin/${arch}/dbghelp.dll
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/dbghelp.dll
            
            COMMAND ${CMAKE_COMMAND}  -E copy_if_different 
                ${IVW_EXTENSIONS_DIR}/vld/bin/${arch}/Microsoft.DTfW.DHL.manifest
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)/Microsoft.DTfW.DHL.manifest    

            DEPENDS 
                ${IVW_EXTENSIONS_DIR}/vld/bin/${arch}/vld_x64.dll
                ${IVW_EXTENSIONS_DIR}/vld/bin/${arch}/dbghelp.dll
                ${IVW_EXTENSIONS_DIR}/vld/bin/${arch}/Microsoft.DTfW.DHL.manifest

            WORKING_DIRECTORY ${OUTPUT_DIR}
            COMMENT "Copying memleak libs"
            VERBATIM
        )
        set_target_properties("copy-memleak-lib" PROPERTIES FOLDER "unittests")

    endif()    
ENDIF()

function(ivw_memleak_setup target)
	if(WIN32 AND MSVC AND IVW_ENABLE_MSVC_MEMLEAK_TEST)
        add_dependencies(${target} copy-memleak-lib)
	endif()
endfunction()