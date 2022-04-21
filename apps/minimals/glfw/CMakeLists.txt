#--------------------------------------------------------------------
# Inviwo GLFW Application
project(inviwo_glfwminimum)

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    glfwminimum.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

ivw_retrieve_all_modules(enabled_modules)
# Remove Qt stuff from list
foreach(module ${enabled_modules})
    string(TOUPPER ${module} u_module)
    if(u_module MATCHES "QT+")
        list(REMOVE_ITEM enabled_modules ${module})
    endif()
endforeach()

# Create application
add_executable(inviwo_glfwminimum MACOSX_BUNDLE WIN32 ${SOURCE_FILES})
target_link_libraries(inviwo_glfwminimum PUBLIC inviwo::core inviwo::module::glfw)
ivw_configure_application_module_dependencies(inviwo_glfwminimum ${enabled_modules})
ivw_define_standard_definitions(inviwo_glfwminimum inviwo_glfwminimum)
ivw_define_standard_properties(inviwo_glfwminimum)

ivw_folder(inviwo_glfwminimum minimals)
ivw_default_install_targets(inviwo_glfwminimum)
