# Inviwo GLFW Application
project(inviwo_glfwminimum)

# Add source files
set(SOURCE_FILES
    glfwminimum.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

set(RES_FILES "")
if(WIN32)
    set(RES_FILES ${RES_FILES} 
        # manifest file for using UTF-8 codepages on Windows
        # see https://learn.microsoft.com/en-us/windows/apps/design/globalizing/use-utf8-code-page
        "${IVW_RESOURCES_DIR}/inviwo.manifest"
    )
endif()
source_group("Resource Files" FILES ${RES_FILES})

# Create application
add_executable(inviwo_glfwminimum MACOSX_BUNDLE WIN32 ${SOURCE_FILES} ${RES_FILES})
target_link_libraries(inviwo_glfwminimum 
    PUBLIC 
        inviwo::core
        inviwo::module-system
        inviwo::module::glfw
)
ivw_define_standard_definitions(inviwo_glfwminimum inviwo_glfwminimum)
ivw_define_standard_properties(inviwo_glfwminimum)

ivw_folder(inviwo_glfwminimum apps)
ivw_default_install_targets(inviwo_glfwminimum)
