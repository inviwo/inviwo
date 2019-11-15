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

#--------------------------------------------------------------------
# Specify standard compile options
# ivw_define_standard_properties(target1 [target2 ...])

option(IVW_TREAT_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(IVW_FORCE_ASSERTIONS "Force use of assertions when not in debug mode" OFF)

function(ivw_define_standard_properties)
    foreach(target ${ARGN})
        get_property(comp_opts TARGET ${target} PROPERTY COMPILE_OPTIONS)
        # Specify warnings
        if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR 
            "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR
            "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
            if(${IVW_TREAT_WARNINGS_AS_ERRORS})
                list(APPEND comp_opts "-Werror") # Threat warnings as errors
            endif()
            list(APPEND comp_opts "-Wall")
            list(APPEND comp_opts "-Wextra")
            list(APPEND comp_opts "-pedantic")
            list(APPEND comp_opts "-Wno-unused-parameter") # not sure we want to remove them.
            list(APPEND comp_opts "-Wno-missing-braces")   # http://stackoverflow.com/questions/13905200/is-it-wise-to-ignore-gcc-clangs-wmissing-braces-warning
        elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
            string(REGEX REPLACE "(^|;)([/-])W[0-9](;|$)" ";" comp_opts "${comp_opts}") # remove any other waning level
            #list(APPEND comp_opts "/nologo") # Suppress Startup Banner
            if(${IVW_TREAT_WARNINGS_AS_ERRORS})
                list(APPEND comp_opts "/WX")     # Threat warnings as errors
            endif()
            list(APPEND comp_opts "/W4")     # Set default warning level to 4
            list(APPEND comp_opts "/wd4005") # macro redefinition    https://msdn.microsoft.com/en-us/library/8d10sc3w.aspx
            list(APPEND comp_opts "/wd4127") # cond expr is const    https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4127
            list(APPEND comp_opts "/wd4201") # nameless struct/union https://msdn.microsoft.com/en-us/library/c89bw853.aspx
            list(APPEND comp_opts "/wd4251") # needs dll-interface   https://msdn.microsoft.com/en-us/library/esew7y1w.aspx
            list(APPEND comp_opts "/wd4505") # unreferenced funtion  https://msdn.microsoft.com/en-us/library/mt694070.aspx
            list(APPEND comp_opts "/w35038") # class member reorder
            if(NOT OpenMP_ON)
                list(APPEND comp_opts "/permissive-")
            endif()
            list(APPEND comp_opts "/std:c++latest")
            #list(APPEND comp_opts "/diagnostics:caret") not supporeted by cmake yet... https://developercommunity.visualstudio.com/content/problem/9385/cmakeliststxt-cannot-override-diagnosticsclassic-d.html
        endif()
        if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR
            "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
            list(APPEND comp_opts "-Wno-mismatched-tags") # gives lots of warnings about redefinitions of structs as class.
        endif()
        if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
            list(APPEND comp_opts "-fsized-deallocation") # see https://github.com/pybind/pybind11/issues/1604
        endif()

        list(REMOVE_DUPLICATES comp_opts)
        set_property(TARGET ${target} PROPERTY COMPILE_OPTIONS ${comp_opts})

        if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
            #https://developer.apple.com/library/mac/documentation/DeveloperTools/Reference/XcodeBuildSettingRef/1-Build_Setting_Reference/build_setting_ref.html
            set_target_properties(${target} PROPERTIES 
                XCODE_ATTRIBUTE_GCC_WARN_NON_VIRTUAL_DESTRUCTOR YES
                XCODE_ATTRIBUTE_GCC_WARN_UNUSED_FUNCTION YES
                XCODE_ATTRIBUTE_GCC_WARN_UNUSED_VARIABLE YES
                XCODE_ATTRIBUTE_GCC_WARN_HIDDEN_VIRTUAL_FUNCTIONS YES
                XCODE_ATTRIBUTE_GCC_WARN_ABOUT_MISSING_FIELD_INITIALIZERS YES
                XCODE_ATTRIBUTE_GCC_WARN_ABOUT_RETURN_TYPE YES
                XCODE_ATTRIBUTE_GCC_WARN_EFFECTIVE_CPLUSPLUS_VIOLATIONS YES
                XCODE_ATTRIBUTE_GCC_WARN_PEDANTIC YES
                XCODE_ATTRIBUTE_GCC_WARN_SHADOW YES
                XCODE_ATTRIBUTE_GCC_WARN_SIGN_COMPARE YES
                XCODE_ATTRIBUTE_CLANG_WARN_ENUM_CONVERSION YES
                XCODE_ATTRIBUTE_WARNING_CFLAGS "-Wunreachable-code"
            )
         endif()
    endforeach()
endfunction()


#--------------------------------------------------------------------
# Define standard defintions
macro(ivw_define_standard_definitions project_name target)
    # Set the compiler flags
    ivw_to_macro_name(u_project_name ${project_name})
    set_target_properties(${target} PROPERTIES DEFINE_SYMBOL ${u_project_name}_EXPORTS)

    target_compile_definitions(${target} PRIVATE 
        $<$<BOOL:${BUILD_SHARED_LIBS}>:INVIWO_ALL_DYN_LINK>
        $<$<BOOL:${IVW_PROFILING}>:IVW_PROFILING>
        $<$<BOOL:${IVW_FORCE_ASSERTIONS}>:IVW_FORCE_ASSERTIONS>
    )
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        target_compile_definitions(${target} PRIVATE 
            _CRT_SECURE_NO_WARNINGS
            _CRT_SECURE_NO_DEPRECATE
            _SCL_SECURE_NO_WARNINGS
            _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
            NOMINMAX
            WIN32_LEAN_AND_MEAN
            UNICODE
            _UNICODE
        )
    else()
        target_compile_definitions(${target} PRIVATE HAVE_CONFIG_H)
    endif()
endmacro()


#--------------------------------------------------------------------
# Suppres all compiler warnings
# ivw_suppress_compiler_warnings(target1 [target2 ...])
function(ivw_suppress_compiler_warnings)
    foreach(target ${ARGN})
        if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR 
            "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR
            "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
            set_property(TARGET ${target} APPEND_STRING PROPERTY COMPILE_FLAGS " -w")

        elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
            get_property(comp_opts TARGET ${target} PROPERTY COMPILE_OPTIONS)
            string(REGEX REPLACE "(^|;)([/-])W[0-9](;|$)" ";" comp_opts "${comp_opts}")
            list(APPEND comp_opts "/W0")
            list(REMOVE_DUPLICATES comp_opts)
            set_property(TARGET ${target} PROPERTY COMPILE_OPTIONS ${comp_opts})
    
            get_property(comp_defs TARGET ${target} PROPERTY COMPILE_DEFINITIONS)
            list(APPEND comp_defs _CRT_SECURE_NO_WARNINGS 
                                  _SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING
                                  _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
                                  _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
                              )
            list(REMOVE_DUPLICATES comp_defs)
            set_property(TARGET ${target} PROPERTY COMPILE_DEFINITIONS ${comp_defs})
            
            set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS " /IGNORE:4006")
        endif()

        if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
            set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_GCC_WARN_INHIBIT_ALL_WARNINGS YES)
        endif()
    endforeach()
endfunction()
