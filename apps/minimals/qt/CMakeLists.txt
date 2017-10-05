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
ivw_retrieve_all_modules(enabled_modules)

#--------------------------------------------------------------------
# Only include one OpenGL context creation module
list(REMOVE_ITEM enabled_modules InviwoGLFWModule)

# Remove modules not used by this app
list(REMOVE_ITEM enabled_modules InviwoPython3QtModule)

#--------------------------------------------------------------------
# Determine dendendencies and set runtime module loading defines
ivw_configure_application_module_dependencies(inviwo_qtminimum
                                              "${enabled_modules}"
                                              dependencies)

# Create application
add_executable(inviwo_qtminimum MACOSX_BUNDLE WIN32 ${SOURCE_FILES})
ivw_register_use_of_modules(inviwo_qtminimum ${enabled_modules})
target_link_libraries(inviwo_qtminimum PUBLIC 
    inviwo::core
    inviwo::qtapplicationbase
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
)
ivw_add_module_dependencies(inviwo_qtminimum ${dependencies})
ivw_define_standard_definitions(inviwo_qtminimum inviwo_qtminimum) 
ivw_define_standard_properties(inviwo_qtminimum)

if(IVW_RUNTIME_MODULE_LOADING)
    target_compile_definitions(inviwo_qtminimum PUBLIC IVW_RUNTIME_MODULE_LOADING)
endif()

#--------------------------------------------------------------------
# Move to folder
ivw_folder(inviwo_qtminimum minimals)
#--------------------------------------------------------------------
# Add application to pack
ivw_default_install_comp_targets(qt_app inviwo_qtminimum)

# Dependencies to build before this project when they are changed.
# Needed if modules are loaded at runtime since they should be built
# when this project is set as startup project
ivw_mod_name_to_target_name(target_names ${enabled_modules})
add_dependencies(inviwo_qtminimum ${target_names})
