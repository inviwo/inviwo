#--------------------------------------------------------------------
# Inviwo GLFW Application
ivw_project(inviwo_glfwminimum)

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    glfwminimum.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})
        
#--------------------------------------------------------------------
# Define libraries that should be linked
ivw_retrieve_all_modules(enabled_modules)
# Core dependencies are linked and will never be loaded at runtime
set(core_dependencies InviwoCore)
list(APPEND core_dependencies GLFW)
list(APPEND core_dependencies InviwoGLFWModule)
list(APPEND enabled_modules ${core_dependencies})

#--------------------------------------------------------------------
# Remove Qt stuff from list
foreach(module ${enabled_modules})
    string(TOUPPER ${module} u_module)
    if(u_module MATCHES "QT+")
        list(REMOVE_ITEM enabled_modules ${module})
    endif()
endforeach()

#--------------------------------------------------------------------
# Register the use of modules
ivw_register_use_of_modules(${enabled_modules})

#--------------------------------------------------------------------
# Determine dendendencies and set runtime module loading defines
set(dependencies "")
ivw_configure_application_module_dependencies("inviwo_glfwminimum" "${core_dependencies}" "${enabled_modules}" dependencies)

# Need to add dependent directories before creating application
ivw_add_dependency_directories(${dependencies})

#--------------------------------------------------------------------
# Create application
add_executable(inviwo_glfwminimum MACOSX_BUNDLE WIN32 ${SOURCE_FILES})

#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(inviwo_glfwminimum inviwo_glfwminimum)

#--------------------------------------------------------------------
# Define standard properties
ivw_define_standard_properties(inviwo_glfwminimum)

#--------------------------------------------------------------------
# Move to folder
ivw_folder(inviwo_glfwminimum minimals)

#--------------------------------------------------------------------
# Add application to pack
ivw_default_install_comp_targets(glfw_app inviwo_glfwminimum)

#--------------------------------------------------------------------
# Add dependencies

# Dependencies to build and link before this project
ivw_add_dependencies(${dependencies})

# Dependencies to build before this project when they are changed.
# Needed if modules are loaded at runtime since they should be built
# when this project is set as startup project
foreach (module_name ${enabled_modules})
    ivw_mod_name_to_target_name(target_name ${module_name})
    add_dependencies(inviwo_glfwminimum "${target_name}")
endforeach()

#--------------------------------------------------------------------
# Optimize compilation with pre-compilied headers based on inviwo-core
ivw_compile_optimize_inviwo_core()
