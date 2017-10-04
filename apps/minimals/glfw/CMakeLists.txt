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
ivw_retrieve_all_modules(package_list)
# Remove Qt stuff from list
foreach(module ${package_list})
    string(TOUPPER ${module} u_module)
    if(u_module MATCHES "QT+")
        list(REMOVE_ITEM package_list ${module})
    endif()
endforeach()

#--------------------------------------------------------------------
# Create application
add_executable(inviwo_glfwminimum MACOSX_BUNDLE WIN32 ${SOURCE_FILES})
ivw_register_use_of_modules(inviwo_glfwminimum ${package_list})
target_link_libraries(inviwo_glfwminimum PUBLIC inviwo-core)
ivw_add_module_dependencies(inviwo_glfwminimum ${package_list})
ivw_define_standard_definitions(inviwo_glfwminimum inviwo_glfwminimum)
ivw_define_standard_properties(inviwo_glfwminimum)

#--------------------------------------------------------------------
# Move to folder
ivw_folder(inviwo_glfwminimum minimals)

#--------------------------------------------------------------------
# Add application to pack
ivw_default_install_comp_targets(glfw_app inviwo_glfwminimum)