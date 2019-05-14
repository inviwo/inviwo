#--------------------------------------------------------------------
# Inviwo Qt Application
project(inviwopyapp)

find_package(Qt5Core QUIET REQUIRED)
find_package(Qt5Gui QUIET REQUIRED)
find_package(Qt5Widgets QUIET REQUIRED)

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    inviwopyapp.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

ivw_retrieve_all_modules(enabled_modules)
# Only include one OpenGL context creation module
list(REMOVE_ITEM enabled_modules InviwoGLFWModule)

# Create application
pybind11_add_module(inviwopyapp ${SOURCE_FILES})

set_target_properties(inviwopyapp PROPERTIES DEBUG_POSTFIX "")
set_target_properties(inviwopyapp PROPERTIES PREFIX "")
set_target_properties(inviwopyapp PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_link_libraries(inviwopyapp PUBLIC 
    inviwo::core
    inviwo::qtapplicationbase
    inviwo::module::qtwidgets
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
)
ivw_configure_application_module_dependencies(inviwopyapp ${enabled_modules})
ivw_define_standard_definitions(inviwopyapp inviwopyapp) 
ivw_define_standard_properties(inviwopyapp)

#--------------------------------------------------------------------
# Move to folder
ivw_folder(inviwopyapp minimals)
#--------------------------------------------------------------------
# Add application to pack
ivw_default_install_comp_targets(qt_app inviwopyapp)
