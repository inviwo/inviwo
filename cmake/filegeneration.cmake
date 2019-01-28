#--------------------------------------------------------------------
# Create a file ("executable_name-enabled-modules.txt") in the binary output directory.
# The application can use the file to check the enabled modules at runtime.
# Usage: ivw_create_enabled_modules_file("application_name" ${enabled_modules})
# where enabled_modules is a list of module names (i.e. InviwoBaseModule)
function(ivw_create_enabled_modules_file executable_name)
    set(enabled_modules "")
    foreach(mod ${ARGN})  
        ivw_mod_name_to_dir(mod_name ${mod})
        set(enabled_modules "${enabled_modules}${mod_name}\n") 
    endforeach()
    if(MSVC OR XCODE_VERSION)
        # Multi-configuration generators (VS, Xcode) append a per-configuration 
        # subdirectory to the specified directory
        foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
            file(WRITE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${OUTPUTCONFIG}/${executable_name}-enabled-modules.txt" ${enabled_modules})
        endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
    else()
        file(WRITE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${executable_name}-enabled-modules.txt" ${enabled_modules})
    endif()
endfunction()

#--------------------------------------------------------------------
# Generate header for modules
function(ivw_generate_module_paths_header)
    set(dirs "")
    foreach(dir ${IVW_MODULE_DIR} ${IVW_EXTERNAL_MODULES})
        if(IS_DIRECTORY ${dir})
            list(APPEND dirs ${dir})
        endif()
    endforeach()

    list_to_longstringvector(vec ${dirs})
    list(LENGTH IVW_EXTERNAL_MODULES count)
    math(EXPR count "${count}+1")
    set(paths "const std::array<const std::string, ${count}> inviwoModulePaths_ = {${vec}}")
    set(IVW_MODULES_PATHS_ARRAY ${paths})

    configure_file(${IVW_CMAKE_TEMPLATES}/inviwomodulespaths_template.h 
                   ${CMAKE_BINARY_DIR}/modules/core/include/inviwo/core/inviwomodulespaths.h @ONLY)
endfunction()

#--------------------------------------------------------------------
# generete python config file
function(ivw_private_create_pyconfig modulepaths activemodules target)
    find_package(Git QUIET)
    if(GIT_FOUND)
        ivw_debug_message(STATUS "git found: ${GIT_EXECUTABLE}")
    else()
        set(GIT_EXECUTABLE "")
    endif()

    string(CONCAT content
        "[Inviwo]\n"
        "modulepaths   = ${modulepaths}\n"
        "activemodules = ${activemodules}\n"
        "binpath       = ${EXECUTABLE_OUTPUT_PATH}\n"
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
    file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/pyconfig-$<CONFIG>.ini CONTENT "[Inviwo]\nexecutable = $<TARGET_FILE:${target}>\n")

endfunction()

#--------------------------------------------------------------------
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

    ivw_private_generate_licence_header(MOD ${mod} RETVAL module_license_vector)
    set(fuction_args
        "        \"${${mod}_name}\", // Module name \n"
        "        \"${${mod}_version}\", // Module version\n"
        "        \"${${mod}_description}\", // Description\n" 
        "        \"${IVW_VERSION}\", // Inviwo core version when built \n" 
        "        ${module_depends_vector}, // Dependencies\n" 
        "        ${module_depends_version_vector}, // Version number of dependencies\n"
        "        ${module_alias_vector}, // List of aliases\n"
        "        // List of license information\n"
        "        ${module_license_vector},\n"
        "        ${module_protected} // protected"
    )

    ivw_join(";" "" fuction_args ${fuction_args})
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


#--------------------------------------------------------------------
# Generate a module registration header files (with configure file etc)
function(ivw_private_generate_module_registration_files modules_var)
    # For runtime loading export a module factory function for all modules.
    # Function will be requested by the application after loading the library (dll/so)
    # Does not require modules to be linked to the application
    # For static loading generate function for creating modules in a single function
    # Requires all modules to be linked to the application

    set(static_headers "")
    set(static_functions "")

    foreach(mod ${${modules_var}})
        if(NOT ${${mod}_opt})
            continue()
        endif()

        list(APPEND static_headers
            "#ifdef REG_${mod}\n"
            "#include <${${mod}_sharedLibInc}>\n"
            "#endif\n"
        )
        list(APPEND static_functions
            "    #ifdef REG_${mod}\n" 
            "    modules.emplace_back(create${${mod}_class}())__SEMICOLON__\n"
            "    #endif\n"
        )
        ivw_private_generate_module_registration_file(${mod})
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
        ${CMAKE_BINARY_DIR}/modules/core/include/inviwo/core/moduleregistration.h 
        @ONLY
    )
endfunction()
