#--------------------------------------------------------------------
# Inviwo GLFW Application
project(inviwo_glfwminimum)

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    glfwminimum.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Define libraries that should be linked
ivw_retrieve_all_modules(enabled_modules)

# Remove Qt stuff from list
foreach(module ${enabled_modules})
    string(TOUPPER ${module} u_module)
    if(u_module MATCHES "QT+")
        list(REMOVE_ITEM enabled_modules ${module})
    endif()
endforeach()

#--------------------------------------------------------------------
# Determine dendendencies and set runtime module loading defines
ivw_configure_application_module_dependencies(inviwo_glfwminimum
                                              "${enabled_modules}"
                                              dependencies)

# Create application
add_executable(inviwo_glfwminimum MACOSX_BUNDLE WIN32 ${SOURCE_FILES})
ivw_register_use_of_modules(inviwo_glfwminimum ${enabled_modules})
target_link_libraries(inviwo_glfwminimum PUBLIC inviwo::core inviwo::module::glfw)
ivw_add_module_dependencies(inviwo_glfwminimum ${dependencies})
ivw_define_standard_definitions(inviwo_glfwminimum inviwo_glfwminimum)
ivw_define_standard_properties(inviwo_glfwminimum)

ivw_folder(inviwo_glfwminimum minimals)
ivw_default_install_comp_targets(glfw_app inviwo_glfwminimum)

# Dependencies to build before this project when they are changed.
# Needed if modules are loaded at runtime since they should be built
# when this project is set as startup project
ivw_mod_name_to_target_name(target_names ${enabled_modules})
add_dependencies(inviwo_glfwminimum ${target_names})
