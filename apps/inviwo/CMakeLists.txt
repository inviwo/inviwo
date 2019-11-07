#--------------------------------------------------------------------
# Inviwo Qt Application
project(inviwo)

find_package(Qt5Core QUIET REQUIRED)
find_package(Qt5Widgets QUIET REQUIRED)

#--------------------------------------------------------------------
# Add MOC files
set(MOC_FILES  ${IVW_APPLICATION_DIR}/inviwo/inviwosplashscreen.h)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES  ${MOC_FILES})
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${IVW_APPLICATION_DIR}/inviwo/inviwo.cpp
    ${IVW_APPLICATION_DIR}/inviwo/inviwosplashscreen.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Add moc files
qt5_wrap_cpp(MOC_FILES ${MOC_FILES})
source_group("MOC Files" FILES ${MOC_FILES})

#--------------------------------------------------------------------
# Define libraries that should be linked
ivw_retrieve_all_modules(enabled_modules)

# Only include one OpenGL context creation module
list(REMOVE_ITEM enabled_modules InviwoGLFWModule)

set(RES_FILES "")
if(WIN32)
    set(RES_FILES ${RES_FILES} "${IVW_RESOURCES_DIR}/inviwo.rc")
elseif(APPLE)
    string(TIMESTAMP YEAR "%Y")
    set(MACOSX_BUNDLE_BUNDLE_NAME          Inviwo)
    set(MACOSX_BUNDLE_GUI_IDENTIFIER       "Inviwo")
    set(MACOSX_BUNDLE_ICON_FILE            inviwo_light.icns)
    set(MACOSX_BUNDLE_INFO_STRING          "Inviwo ${IVW_VERSION}, Copyright (c) 2012-${YEAR} Inviwo Foundation")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${IVW_VERSION}")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING  "${IVW_VERSION}.")
    set(MACOSX_BUNDLE_BUNDLE_VERSION       "${IVW_VERSION}")
    set(MACOSX_BUNDLE_COPYRIGHT            "(C) 2012-${YEAR} Inviwo Foundation")
    set_source_files_properties(${IVW_ROOT_DIR}/resources/inviwo/inviwo_light.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(RES_FILES ${RES_FILES} ${IVW_ROOT_DIR}/resources/inviwo/inviwo_light.icns)
endif()
source_group("Resource Files" FILES ${RES_FILES})

#--------------------------------------------------------------------
# Create application
add_executable(inviwo MACOSX_BUNDLE WIN32 
    ${SOURCE_FILES} 
    ${HEADER_FILES} 
    ${MOC_FILES} 
    ${RES_FILES})
target_link_libraries(inviwo PUBLIC 
    inviwo::core
    inviwo::qtapplicationbase
    inviwo::qteditor
    Qt5::Core
    Qt5::Widgets
)
#--------------------------------------------------------------------
# Register the use of modules
# Determine dendendencies and set runtime module loading defines
ivw_configure_application_module_dependencies(inviwo ${enabled_modules})
ivw_define_standard_definitions(inviwo inviwo)
ivw_define_standard_properties(inviwo)

#--------------------------------------------------------------------
# Add application to pack
if(WIN32)
    install(TARGETS inviwo RUNTIME DESTINATION bin COMPONENT qt_app)

elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    #Following http://www.cmake.org/Wiki/BundleUtilitiesExample
    install(
        TARGETS inviwo 
        RUNTIME   DESTINATION bin 
        BUNDLE    DESTINATION . 
        ARCHIVE   DESTINATION Inviwo.app/Contents/MacOS/ 
        LIBRARY   DESTINATION Inviwo.app/Contents/MacOS/ 
        COMPONENT qt_app
    )
    set(plugin_dest_dir Inviwo.app/Contents/MacOS)
    set(qtconf_dest_dir Inviwo.app/Contents/Resources)
    set(APPS "\${CMAKE_INSTALL_PREFIX}/Inviwo.app/Contents/MacOS/Inviwo")
    set(DIRS "\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/")
    
    # get link errors with out this line.
    install(CODE "file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"\")" COMPONENT qt_app)
    install(CODE "set(BU_CHMOD_BUNDLE_ITEMS ON)
                  include(BundleUtilities)
                  file(GLOB_RECURSE LIBS1 \"\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}\")
                  file(GLOB_RECURSE LIBS2 \"\${CMAKE_INSTALL_PREFIX}/Inviwo.app/Contents/plugins/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
                  fixup_bundle(\"${APPS}\" \"\${LIBS1};\${LIBS2}\" \"${DIRS}\")"
            COMPONENT qt_app
    )
    install(
        DIRECTORY   ${IVW_ROOT_DIR}/data/ 
        DESTINATION Inviwo.app/Contents/Resources/data
        COMPONENT   qt_app
    )

    set(CPACK_BINARY_DRAGNDROP ON)

else()
    install(
        TARGETS   inviwo
        RUNTIME   DESTINATION bin
        BUNDLE    DESTINATION bin
        ARCHIVE   DESTINATION lib
        LIBRARY   DESTINATION lib
        COMPONENT qt_app
    )
endif()

# Add build dependency towards any pybind11 targets and inviwo,
# if python is not enabled, this list will be empty
if(_allPyBindWrappers)
    add_dependencies(inviwo ${_allPyBindWrappers})
endif()
ivw_memleak_setup(inviwo)

#--------------------------------------------------------------------
# add a console version of inviwo.exe called inviwo-cli.exe
if(WIN32)
    set_property(TARGET inviwo APPEND_STRING PROPERTY LINK_FLAGS 
        " /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")

    project(inviwo-cli)

    add_executable(inviwo-cli MACOSX_BUNDLE WIN32 
        ${SOURCE_FILES} 
        ${HEADER_FILES} 
        ${MOC_FILES} 
        ${QRC_FILE} 
        ${RES_FILES}
    )
    target_link_libraries(inviwo-cli PUBLIC
        inviwo::core 
        inviwo::qtapplicationbase
        inviwo::qteditor
        Qt5::Core 
        Qt5::Widgets
    )
    ivw_configure_application_module_dependencies(inviwo-cli ${enabled_modules})
    ivw_define_standard_definitions(inviwo-cli inviwo-cli)
    ivw_define_standard_properties(inviwo-cli)

    if(_allPyBindWrappers)
        add_dependencies(inviwo-cli ${_allPyBindWrappers})
    endif()

    # prevent inviwo-cli from building in parallel with inviwo since that might cause errors with the moc generation
    add_dependencies(inviwo-cli inviwo)

    ivw_memleak_setup(inviwo-cli)

    set_property(TARGET inviwo-cli APPEND_STRING PROPERTY LINK_FLAGS 
        " /SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup")

    install(TARGETS inviwo-cli RUNTIME DESTINATION bin COMPONENT qt_app)
endif()

# Save information for python tools.
ivw_mod_name_to_class(ivw_module_classes ${enabled_modules})
ivw_private_create_pyconfig("${IVW_MODULE_DIR};${IVW_EXTERNAL_MODULES}" "${ivw_module_classes}" inviwo)
