#--------------------------------------------------------------------
# Inviwo Qt Application
project(inviwo_configurable)

find_package(Qt5Core QUIET REQUIRED)
find_package(Qt5Gui QUIET REQUIRED)
find_package(Qt5Widgets QUIET REQUIRED)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    configparser.h
    gui.h
)
ivw_group("Header Files" ${HEADER_FILES})
# Add source files
set(SOURCE_FILES
    configparser.cpp
    gui.cpp
    main.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

ivw_retrieve_all_modules(enabled_modules)
# Only include one OpenGL context creation module
list(REMOVE_ITEM enabled_modules InviwoGLFWModule)
# Remove modules not used by this app
list(REMOVE_ITEM enabled_modules InviwoPython3QtModule)

# Create application
add_executable(inviwo_configurable MACOSX_BUNDLE WIN32 ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(inviwo_configurable PUBLIC 
    inviwo::core
    inviwo::qtapplicationbase
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
)
ivw_configure_application_module_dependencies(inviwo_configurable ${enabled_modules})
ivw_define_standard_definitions(inviwo_configurable inviwo_configurable) 
ivw_define_standard_properties(inviwo_configurable)

#--------------------------------------------------------------------
# Move to folder
ivw_folder(inviwo_configurable configurable)
#--------------------------------------------------------------------
# Add application to pack
ivw_default_install_comp_targets(qt_app inviwo_configurable)
