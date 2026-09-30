# Inviwo Qt Application
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

if(WIN32)
    add_library(inviwo-objs OBJECT ${SOURCE_FILES} ${HEADER_FILES} ${MOCED_FILES})
    target_link_libraries(inviwo-objs PUBLIC
        inviwo::core
        inviwo::module-system
        inviwo::python-helper
        inviwo::qtapplicationbase
        inviwo::qteditor
        Qt6::Core
        Qt6::Widgets
    )
    ivw_define_standard_definitions(inviwo-objs inviwo-objs)
    ivw_define_standard_properties(QT inviwo-objs)

    set(RES_FILES
        "${IVW_RESOURCES_DIR}/inviwo.rc"
        # manifest file for using UTF-8 codepages on Windows
        # see https://learn.microsoft.com/en-us/windows/apps/design/globalizing/use-utf8-code-page
        "${IVW_RESOURCES_DIR}/inviwo.manifest"
    )
    source_group("Resource Files" FILES ${RES_FILES})

    # add a windows (inviwo.exe) and a console version (inviwo-cli.exe) of inviwo
    add_executable(inviwo WIN32 ${RES_FILES})
    target_link_libraries(inviwo PUBLIC inviwo-objs)
    set_property(TARGET inviwo APPEND_STRING PROPERTY LINK_FLAGS
        " /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
    ivw_default_install_targets(inviwo)

    add_executable(inviwo-cli WIN32 ${RES_FILES})
    target_link_libraries(inviwo-cli PUBLIC inviwo-objs)
    set_property(TARGET inviwo-cli APPEND_STRING PROPERTY LINK_FLAGS
        " /SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup")
    ivw_default_install_targets(inviwo-cli)

    if(pymodules)
        add_dependencies(inviwo-cli ${pymodules})
    endif()

elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_source_files_properties(${IVW_RESOURCES_DIR}/inviwo/inviwo_light.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(RES_FILES ${IVW_RESOURCES_DIR}/inviwo/inviwo_light.icns)
    source_group("Resource Files" FILES ${RES_FILES})

    add_executable(inviwo MACOSX_BUNDLE ${RES_FILES} ${SOURCE_FILES} ${HEADER_FILES} ${MOCED_FILES})
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
    ivw_default_install_targets(inviwo)

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
        XCODE_GENERATE_SCHEME YES
    )

else()
    add_executable(inviwo ${SOURCE_FILES} ${HEADER_FILES} ${MOCED_FILES})
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
    ivw_default_install_targets(inviwo)

    if("qt" IN_LIST VCPKG_MANIFEST_FEATURES)
        if(NOT BUILD_SHARED_LIBS)
            qt_import_plugins(
                inviwo
                INCLUDE
                    Qt::QXcbIntegrationPlugin
                    Qt::QMinimalIntegrationPlugin
                    Qt::QOffscreenIntegrationPlugin
                    Qt::QLinuxFbIntegrationPlugin
                    Qt::QGifPlugin
                    Qt::QJpegPlugin
                    Qt::QSvgPlugin
            )
        endif()
    endif()
endif()

option(IVW_CFG_TRACY_MENORY_PROFILING "Enable tracy memory profiling settings" OFF)
if(IVW_CFG_TRACY_MENORY_PROFILING)
    target_compile_definitions(inviwo PUBLIC IVW_CFG_TRACY_MENORY_PROFILING)
endif()

ivw_deploy_qt(inviwo)

if(IVW_PACKAGE_SELECT_APP STREQUAL "inviwo")
    include(packaging/packaging.cmake) # Package creation
endif()

ivw_private_create_pyconfig_exe(TARGET inviwo)
