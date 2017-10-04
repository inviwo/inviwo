#--------------------------------------------------------------------
# Inviwo Qt Application
project(inviwo_qtminimum)

find_package(Qt5Core QUIET REQUIRED)
find_package(Qt5Gui QUIET REQUIRED)
find_package(Qt5Widgets QUIET REQUIRED)

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    qtminimum.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Define libraries that should be linked
ivw_retrieve_all_modules(package_list)
# Only include one OpenGL context creation module
list(REMOVE_ITEM package_list InviwoGLFWModule)
# Remove modules not used by this app
list(REMOVE_ITEM package_list InviwoPython3QtModule)

#--------------------------------------------------------------------
# Create application
add_executable(inviwo_qtminimum MACOSX_BUNDLE WIN32 ${SOURCE_FILES})
ivw_register_use_of_modules(inviwo_qtminimum ${package_list})
target_link_libraries(inviwo_qtminimum PUBLIC 
    inviwo::core
    inviwo::qtapplicationbase
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
)
ivw_add_module_dependencies(inviwo_qtminimum ${package_list})
ivw_define_standard_definitions(inviwo_qtminimum inviwo_qtminimum) 
ivw_define_standard_properties(inviwo_qtminimum)

#--------------------------------------------------------------------
# Move to folder
ivw_folder(inviwo_qtminimum minimals)
#--------------------------------------------------------------------
# Add application to pack
ivw_default_install_comp_targets(qt_app inviwo_qtminimum)

