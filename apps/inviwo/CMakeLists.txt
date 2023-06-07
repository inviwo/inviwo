# Inviwo Qt Application
project(inviwo)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Add MOC files
set(MOC_FILES ${IVW_APPLICATION_DIR}/inviwo/inviwosplashscreen.h)

# Add header files
set(HEADER_FILES ${MOC_FILES})
ivw_group("Header Files" ${HEADER_FILES})

# Add source files
set(SOURCE_FILES
    ${IVW_APPLICATION_DIR}/inviwo/inviwo.cpp
    ${IVW_APPLICATION_DIR}/inviwo/inviwosplashscreen.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Add moc files
qt_wrap_cpp(MOCED_FILES ${MOC_FILES})
source_group("MOC Files" FILES ${MOCED_FILES})

# Define libraries that should be linked
ivw_retrieve_all_modules(enabled_modules)

# Only include one OpenGL context creation module
list(REMOVE_ITEM enabled_modules InviwoGLFWModule)

set(RES_FILES "")
if(WIN32)
    set(RES_FILES ${RES_FILES} 
        "${IVW_RESOURCES_DIR}/inviwo.rc"
        # manifest file for using UTF-8 codepages on Windows
        # see https://learn.microsoft.com/en-us/windows/apps/design/globalizing/use-utf8-code-page
        "${IVW_RESOURCES_DIR}/inviwo.manifest"
    )
elseif(APPLE)
    set_source_files_properties(${IVW_ROOT_DIR}/resources/inviwo/inviwo_light.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(RES_FILES ${RES_FILES} ${IVW_ROOT_DIR}/resources/inviwo/inviwo_light.icns)
endif()
source_group("Resource Files" FILES ${RES_FILES})

# Create application
add_executable(inviwo MACOSX_BUNDLE WIN32 
    ${SOURCE_FILES} 
    ${HEADER_FILES} 
    ${MOCED_FILES} 
    ${RES_FILES})
target_link_libraries(inviwo PUBLIC 
    inviwo::core
    inviwo::qtapplicationbase
    inviwo::qteditor
    Qt6::Core
    Qt6::Widgets
)

set_target_properties(inviwo PROPERTIES OUTPUT_NAME "Inviwo")

# Register the use of modules
# Determine dendendencies and set runtime module loading defines
ivw_configure_application_module_dependencies(inviwo ${enabled_modules})
ivw_define_standard_definitions(inviwo inviwo)
ivw_define_standard_properties(QT inviwo)

# Add application to pack
ivw_default_install_targets(inviwo)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # Configure Info.plist
    # https://cmake.org/cmake/help/v3.15/prop_tgt/MACOSX_BUNDLE_INFO_PLIST.html
    string(TIMESTAMP YEAR "%Y")
    set_target_properties(inviwo PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${IVW_RESOURCES_DIR}/MacOSXBundleInfo.plist.in"
        MACOSX_BUNDLE_INFO_STRING "Inviwo ${IVW_VERSION}, Copyright (c) 2012-${YEAR} Inviwo Foundation"
        MACOSX_BUNDLE_ICON_FILE "inviwo_light.icns"
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.inviwo.Inviwo"
        MACOSX_BUNDLE_LONG_VERSION_STRING "${IVW_VERSION}"
        MACOSX_BUNDLE_BUNDLE_NAME "Inviwo"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${IVW_VERSION}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${IVW_VERSION}"
        MACOSX_BUNDLE_COPYRIGHT "C) 2012-${YEAR} Inviwo Foundation"
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "org.inviwo.Inviwo"
    )

    set(CPACK_BINARY_DRAGNDROP ON)
endif()


# Add build dependency towards any pybind11 targets and inviwo,
# if python is not enabled, this list will be empty

get_property(pymodules GLOBAL PROPERTY IVW_PYMODULE_LIST)
if(pymodules)
    add_dependencies(inviwo ${pymodules})
endif()

# add a console version of inviwo.exe called inviwo-cli.exe
if(WIN32)
    set_property(TARGET inviwo APPEND_STRING PROPERTY LINK_FLAGS 
        " /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")

    project(inviwo-cli)

    add_executable(inviwo-cli MACOSX_BUNDLE WIN32 
        ${SOURCE_FILES} 
        ${HEADER_FILES} 
        ${MOCED_FILES} 
        ${QRC_FILE} 
        ${RES_FILES}
    )
    target_link_libraries(inviwo-cli PUBLIC
        inviwo::core 
        inviwo::qtapplicationbase
        inviwo::qteditor
        Qt6::Core 
        Qt6::Widgets
    )
    set_target_properties(inviwo-cli PROPERTIES OUTPUT_NAME "Inviwo-cli")
    ivw_configure_application_module_dependencies(inviwo-cli ${enabled_modules})
    ivw_define_standard_definitions(inviwo-cli inviwo-cli)
    ivw_define_standard_properties(inviwo-cli)

    if(pymodules)
        add_dependencies(inviwo-cli ${pymodules})
    endif()

    # prevent inviwo-cli from building in parallel with inviwo since that might cause errors with the moc generation
    add_dependencies(inviwo-cli inviwo)

    set_property(TARGET inviwo-cli APPEND_STRING PROPERTY LINK_FLAGS 
        " /SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup")

    ivw_default_install_targets(inviwo-cli)
endif()

# Save information for python tools.
ivw_mod_name_to_class(ivw_module_classes ${enabled_modules})
ivw_private_create_pyconfig("${IVW_MODULE_DIR};${IVW_EXTERNAL_MODULES}" "${ivw_module_classes}" inviwo)
ivw_deploy_qt(inviwo)

if(IVW_PACKAGE_SELECT_APP STREQUAL "inviwo")
    include(packaging/packaging.cmake) # Package creation
endif()
