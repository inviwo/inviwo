#################################################################################
# 
# Inviwo - Interactive Visualization Workshop
# 
# Copyright (c) 2023-2024 Inviwo Foundation
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

# Create a file ("executable_name-enabled-modules.txt") in the binary output directory.
# The application can use the file to check the enabled modules at runtime.
# Usage: ivw_create_enabled_modules_file("application_name" ${enabled_modules})
# where enabled_modules is a list of module names (i.e. InviwoBaseModule)
function(ivw_create_enabled_modules_file executable_name)
    set(enabled_modules "")
    foreach(mod IN LISTS ARGN)
        ivw_mod_name_to_dir(mod_name ${mod})
        set(enabled_modules "${enabled_modules}${mod_name}\n") 
    endforeach()
    if(MSVC OR XCODE_VERSION)
        # Multi-configuration generators (VS, Xcode) append a per-configuration 
        # subdirectory to the specified directory
        foreach(OUTPUTCONFIG IN LISTS CMAKE_CONFIGURATION_TYPES)
            file(WRITE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${OUTPUTCONFIG}/${executable_name}-enabled-modules.txt" ${enabled_modules})
        endforeach()
    else()
        file(WRITE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${executable_name}-enabled-modules.txt" ${enabled_modules})
    endif()
endfunction()

# generete python config file
function(ivw_private_create_pyconfig)
    set(options )
    set(oneValueArgs TARGET)
    set(multiValueArgs MODULE_DIRS ENABLED_MODULES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "ivw_private_create_pyconfig: Unparsed arguments ${ARG_UNPARSED_ARGUMENTS}")
    endif()
    if(ARG_KEYWORDS_MISSING_VALUES) 
        message(FATAL_ERROR "ivw_private_create_pyconfig: Missing values for keywords ${ARG_KEYWORDS_MISSING_VALUES}")
    endif()
    if(NOT ARG_MODULE_DIRS)
        message(FATAL_ERROR "ivw_private_create_pyconfig: MODULE_DIRS not specified")
    endif()
    if(NOT ARG_ENABLED_MODULES)
        message(FATAL_ERROR "ivw_private_create_pyconfig: ENABLED_MODULES not specified")
    endif()
    if(NOT ARG_TARGET)
        message(FATAL_ERROR "ivw_private_create_pyconfig: TARGET not specified")
    endif()

    find_package(Git QUIET)
    if(GIT_FOUND)
        ivw_debug_message(STATUS "git found: ${GIT_EXECUTABLE}")
    else()
        set(GIT_EXECUTABLE "")
    endif()

    string(CONCAT content
        "[Inviwo]\n"
        "modulepaths   = ${ARG_MODULE_DIRS}\n"
        "activemodules = ${ARG_ENABLED_MODULES}\n"
        "binpath       = ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}\n"
        "builds        = ${CMAKE_CONFIGURATION_TYPES}\n"
        "\n"
        "[CMake]\n"
        "path          = ${CMAKE_COMMAND}\n"
        "binary_dir    = ${CMAKE_BINARY_DIR}\n"
        "source_dir    = ${CMAKE_SOURCE_DIR}\n"
        "\n"
        "[Git]\n"
        "path          = ${GIT_EXECUTABLE}\n")

    file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/pyconfig.ini CONTENT "${content}")
    file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/pyconfig-$<CONFIG>.ini CONTENT "[Inviwo]\nexecutable = $<TARGET_FILE:${ARG_TARGET}>\n")
endfunction()

# Generate a module registration header file (with configure file etc)
function(ivw_private_generate_module_registration_file mod)
    ivw_mod_name_to_dir(module_dependencies ${${mod}_dependencies})
    list_to_stringvector(module_depends_vector ${module_dependencies})
    list_to_stringvector(module_depends_version_vector ${${mod}_dependenciesversion})
    list_to_stringvector(module_alias_vector ${${mod}_aliases})
    if(${${mod}_protected})
        set(module_protected "ProtectedModule::on")
    else()
        set(module_protected "ProtectedModule::off")
    endif()

    ivw_private_generate_license_header(MOD ${mod} RETVAL module_license_vector)
    string(CONCAT fuction_args
        "        \"${${mod}_name}\", // Module name \n"
        "        \"${${mod}_version}\", // Module version\n"
        "        \"${${mod}_description}\", // Description\n"
        "        \"${${mod}_path}\", // Module root dir\n"
        "        \"${IVW_VERSION}\", // Inviwo core version when built \n"
        "        ${module_depends_vector}, // Dependencies\n"
        "        ${module_depends_version_vector}, // Version number of dependencies\n"
        "        ${module_alias_vector}, // List of aliases\n"
        "        // List of license information\n"
        "        ${module_license_vector},\n"
        "        ${module_protected} // protected"
    )
    string(REPLACE "__LINEBREAK__" "\\n\"\n        \"" fuction_args "${fuction_args}")
    string(REPLACE "__SEMICOLON__" ";" fuction_args "${fuction_args}")

    set(MODULE_CLASS "${${mod}_class}")
    set(API_HEADER ${${mod}_apiDefineInc})
    set(API_DEFINE ${${mod}_api})
    set(MODULE_HEADER ${${mod}_header})
    set(MODULE_ARGS ${fuction_args})
    set(LIBRARY_HEADER ${${mod}_sharedLibInc})

    configure_file(
        ${IVW_CMAKE_TEMPLATES}/mod_shared_library_template.cpp 
        ${${mod}_sharedLibCpp}
        @ONLY
    )
    configure_file(
        ${IVW_CMAKE_TEMPLATES}/mod_shared_library_template.h
        ${${mod}_sharedLibHpp}
        @ONLY
    )
endfunction()

# Generate a module registration header files (with configure file etc)
function(ivw_private_generate_module_registration_files)
    # For runtime loading export a module factory function for all modules.
    # Function will be requested by the application after loading the library (dll/so)
    # Does not require modules to be linked to the application
    # For static loading generate function for creating modules in a single function
    # Requires all modules to be linked to the application

    set(options )
    set(oneValueArgs DESTINATION)
    set(multiValueArgs MODULES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ARG_DESTINATION)
        message(FATAL_ERROR "ivw_private_generate_module_registration_files: DESTINATION not specified")
    endif()
    if(NOT ARG_MODULES)
        message(FATAL_ERROR "ivw_private_generate_module_registration_files: MODULES not specified")
    endif()

    set(static_headers "")
    set(static_functions "")

    foreach(mod IN LISTS ARG_MODULES)
        list(APPEND static_headers
            "#include <${${mod}_sharedLibInc}>\n"
        )
        list(APPEND static_functions
            "    modules.emplace_back(create${${mod}_class}())__SEMICOLON__\n"
        )
    endforeach()

    ivw_join(";" "" static_headers ${static_headers})
    ivw_join(";" "" static_functions ${static_functions})
    string(REPLACE "__LINEBREAK__" "\\n\"\n        \"" static_functions "${static_functions}")
    string(REPLACE "__SEMICOLON__" ";" static_functions "${static_functions}")

    set(MODULE_HEADERS "${static_headers}")
    set(MODULE_CLASS_FUNCTIONS "${static_functions}")
    # uses: MODULE_HEADERS MODULE_CLASS_FUNCTIONS
    configure_file(
        ${IVW_CMAKE_TEMPLATES}/mod_registration_template.h 
        ${ARG_DESTINATION}
        @ONLY
    )
endfunction()
