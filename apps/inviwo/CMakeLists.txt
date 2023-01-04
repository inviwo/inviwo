# Inviwo Qt Application
project(inviwo)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Add MOC files
set(MOC_FILES inviwosplashscreen.h)

# Add header files
set(HEADER_FILES ${MOC_FILES})
ivw_group("Header Files" ${HEADER_FILES})

# Add source files
set(SOURCE_FILES
    inviwo.cpp
    inviwosplashscreen.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Add moc files
qt_wrap_cpp(MOCED_FILES ${MOC_FILES})
source_group("MOC Files" FILES ${MOCED_FILES})

set(RES_FILES "")
if(WIN32)
    list(APPEND RES_FILES
        "${IVW_RESOURCES_DIR}/inviwo.rc"
        # manifest file for using UTF-8 codepages on Windows
        # see https://learn.microsoft.com/en-us/windows/apps/design/globalizing/use-utf8-code-page
        "${IVW_RESOURCES_DIR}/inviwo.manifest"
    )
elseif(APPLE)
    set_source_files_properties(${IVW_RESOURCES_DIR}/inviwo/inviwo_light.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    list(APPEND RES_FILES ${IVW_RESOURCES_DIR}/inviwo/inviwo_light.icns)
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
    inviwo::module-system
    inviwo::python-helper
    inviwo::qtapplicationbase
    inviwo::qteditor
    Qt6::Core
    Qt6::Widgets
)

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
        inviwo::module-system
        inviwo::qtapplicationbase
        inviwo::qteditor
        Qt6::Core 
        Qt6::Widgets
    )

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

ivw_deploy_qt(inviwo)

if(IVW_PACKAGE_SELECT_APP STREQUAL "inviwo")
    include(packaging/packaging.cmake) # Package creation
endif()

ivw_private_create_pyconfig_exe(TARGET inviwo)
