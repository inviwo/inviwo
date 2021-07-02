# Inviwo Qt Application
project(inviwo_qtminimum)

# https://doc-snapshots.qt.io/qt6-dev/cmake-qt5-and-qt6-compatibility.html#supporting-older-qt-5-versions
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets REQUIRED)


# Add source files
set(SOURCE_FILES
    qtminimum.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

set(RES_FILES "")
if(WIN32)
    set(RES_FILES ${RES_FILES} "${IVW_RESOURCES_DIR}/inviwo.rc")
elseif(APPLE)
    set_source_files_properties(${IVW_ROOT_DIR}/resources/inviwo/inviwo_light.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(RES_FILES ${RES_FILES} ${IVW_ROOT_DIR}/resources/inviwo/inviwo_light.icns)
endif()
source_group("Resource Files" FILES ${RES_FILES})

ivw_retrieve_all_modules(enabled_modules)
# Only include one OpenGL context creation module
list(REMOVE_ITEM enabled_modules InviwoGLFWModule)
# Remove modules not used by this app
list(REMOVE_ITEM enabled_modules InviwoPython3QtModule)

# Create application
add_executable(inviwo_qtminimum MACOSX_BUNDLE WIN32 ${SOURCE_FILES} ${RES_FILES})
target_link_libraries(inviwo_qtminimum PUBLIC 
    inviwo::core
    inviwo::qtapplicationbase
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
)
target_compile_definitions(inviwo_qtminimum PUBLIC 
    "QT_DEPRECATED_WARNINGS_SINCE=QT_VERSION_CHECK(5, 15, 0)"
)
ivw_configure_application_module_dependencies(inviwo_qtminimum ${enabled_modules})
ivw_define_standard_definitions(inviwo_qtminimum inviwo_qtminimum) 
ivw_define_standard_properties(inviwo_qtminimum)

# Move to folder
ivw_folder(inviwo_qtminimum minimals)
# Add application to pack
ivw_default_install_comp_targets(qt_app inviwo_qtminimum)
