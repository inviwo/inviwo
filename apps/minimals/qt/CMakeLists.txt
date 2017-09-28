#--------------------------------------------------------------------
# Inviwo Qt Application
ivw_project(inviwo_qtminimum)

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

set(core_dependencies InviwoCore)
# Core dependencies are linked and will never be loaded at runtime
list(APPEND core_dependencies InviwoQtApplicationBase)
list(APPEND enabled_modules ${core_dependencies})

#--------------------------------------------------------------------
# Only include one OpenGL context creation module
list(REMOVE_ITEM enabled_modules InviwoGLFWModule)

#--------------------------------------------------------------------
# Remove modules not used by this app
list(REMOVE_ITEM enabled_modules InviwoPython3QtModule)

#--------------------------------------------------------------------
# Register the use of modules
ivw_register_use_of_modules(${enabled_modules})

#--------------------------------------------------------------------
# Determine dendendencies and set runtime module loading defines
set(dependencies "")
ivw_configure_application_module_dependencies("inviwo_qtminimum" "${core_dependencies}" "${enabled_modules}" dependencies)

# Need to add dependent directories before creating application
ivw_add_dependency_directories(${dependencies})

#--------------------------------------------------------------------
# Create application
add_executable(inviwo_qtminimum MACOSX_BUNDLE WIN32 ${SOURCE_FILES})

#--------------------------------------------------------------------
# Add qt dependencies to this list
ivw_add_dependencies(Qt5Core Qt5Gui Qt5Widgets)
ivw_define_qt_definitions()

#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(inviwo_qtminimum inviwo_qtminimum) 
#--------------------------------------------------------------------
# Define standard properties
ivw_define_standard_properties(inviwo_qtminimum)

#--------------------------------------------------------------------
# Move to folder
ivw_folder(inviwo_qtminimum minimals)

#--------------------------------------------------------------------
# Add application to pack
ivw_default_install_comp_targets(qt_app inviwo_qtminimum)

#--------------------------------------------------------------------
# Add dependencies

# Dependencies to build and link before this project
ivw_add_dependencies(${dependencies})

# Dependencies to build before this project when they are changed.
# Needed if modules are loaded at runtime since they should be built
# when this project is set as startup project
foreach (module_name ${enabled_modules})
    ivw_mod_name_to_target_name(target_name ${module_name})
    add_dependencies(inviwo_qtminimum "${target_name}")
endforeach()

#--------------------------------------------------------------------
# Optimize compilation with pre-compilied headers based on inviwo-core
ivw_compile_optimize_inviwo_core()
