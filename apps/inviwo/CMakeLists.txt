#--------------------------------------------------------------------
# Inviwo Qt Application
ivw_project(inviwo)

if(POLICY CMP0043)
    cmake_policy(SET CMP0043 OLD) # Ignore compile definitions.
endif()


#--------------------------------------------------------------------
# Add MOC files
set(MOC_FILES
    ${IVW_APPLICATION_DIR}/inviwo/inviwosplashscreen.h
)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${MOC_FILES}
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${IVW_APPLICATION_DIR}/inviwo/inviwo.cpp
    ${IVW_APPLICATION_DIR}/inviwo/inviwosplashscreen.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

ivw_qt_wrap_cpp(MOC_FILES ${HEADER_FILES})
source_group("MOC Files" FILES ${MOC_FILES})
source_group("Resource Files" FILES ${IVW_RESOURCES_DIR}/inviwo.qrc)

#--------------------------------------------------------------------
# Add resource file
if(DESIRED_QT_VERSION MATCHES 5)
    qt5_add_resources(QRC_FILE "${IVW_RESOURCES_DIR}/inviwo.qrc")
else()
    qt4_add_resources(QRC_FILE "${IVW_RESOURCES_DIR}/inviwo.qrc")
endif()
        
#--------------------------------------------------------------------
# Define libraries that should be linked
ivw_retrieve_all_modules(package_list)
list(APPEND package_list InviwoQtEditor)
list(APPEND package_list InviwoCore)
# Only include one OpenGL context creation module
if(IVW_USE_GLFW_NOT_OPENGLQT)
    list(REMOVE_ITEM package_list InviwoOpenGLQtModule)
else()
    list(REMOVE_ITEM package_list InviwoGLFWModule)
endif()

#--------------------------------------------------------------------
# Remove modules not used by this app
list(REMOVE_ITEM package_list InviwoGLUTModule)
list(REMOVE_ITEM package_list InviwoSDLModule)

#--------------------------------------------------------------------
# Register the use of modules
ivw_register_use_of_modules(${package_list})

#--------------------------------------------------------------------
# Need to add dependent directories before creating application
ivw_add_dependency_directories(${package_list})

set(RES_FILES "")
if(WIN32)
    set(RES_FILES ${RES_FILES} "${IVW_RESOURCES_DIR}/inviwo.rc")
elseif(APPLE)
    set(MACOSX_BUNDLE_BUNDLE_NAME Inviwo)
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "Inviwo")
    set(MACOSX_BUNDLE_ICON_FILE inviwo_light.icns)
    set(MACOSX_BUNDLE_INFO_STRING "Inviwo ${IVW_VERSION}, Copyright (c) 2012-2015 Inviwo Foundation")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${IVW_VERSION}")
    set(MACOSX_BUNDLE_LONG_VERSION_STRING "${IVW_VERSION}.")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "${IVW_VERSION}")
    set(MACOSX_BUNDLE_COPYRIGHT "(C) 2012-2015 Inviwo Foundation")
    set_source_files_properties(${IVW_ROOT_DIR}/resources/icons/inviwo_light.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(RES_FILES ${RES_FILES} ${IVW_ROOT_DIR}/resources/icons/inviwo_light.icns)
endif()

#--------------------------------------------------------------------
# Create application
add_executable(inviwo MACOSX_BUNDLE WIN32 ${SOURCE_FILES} ${HEADER_FILES} ${MOC_FILES} ${QRC_FILE} ${RES_FILES})

#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(inviwo inviwo)

#--------------------------------------------------------------------
# Define standard properties
ivw_define_standard_properties(inviwo)

#--------------------------------------------------------------------
# Add application to pack
if(IVW_PACKAGE_PROJECT)
    if(WIN32)
        install(TARGETS inviwo
                RUNTIME DESTINATION bin
                COMPONENT qt_app)

    elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        #Following http://www.cmake.org/Wiki/BundleUtilitiesExample

        install(TARGETS inviwo 
            RUNTIME DESTINATION bin 
            BUNDLE DESTINATION . 
            ARCHIVE DESTINATION Inviwo.app/Contents/MacOS/ 
            LIBRARY DESTINATION Inviwo.app/Contents/MacOS/ 
            COMPONENT qt_app
        )

        set(plugin_dest_dir Inviwo.app/Contents/MacOS)
        set(qtconf_dest_dir Inviwo.app/Contents/Resources)
        set(APPS "\${CMAKE_INSTALL_PREFIX}/Inviwo.app/Contents/MacOS/Inviwo")
        set(DIRS "${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/")
        
        # get link errors with out this line.
        install(CODE "file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"\")" COMPONENT qt_app)
        
        #install(
        #    DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib/
        #    DESTINATION ${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}
        #    COMPONENT qt_app
        #)
        install(CODE "set(BU_CHMOD_BUNDLE_ITEMS ON)
                      include(BundleUtilities)
                      file(GLOB_RECURSE LIBS1 \"\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}\")
                      file(GLOB_RECURSE LIBS2 \"\${CMAKE_INSTALL_PREFIX}/Inviwo.app/Contents/plugins/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
                      fixup_bundle(\"${APPS}\" \"\${LIBS1};\${LIBS2}\" \"${DIRS}\")"
                COMPONENT qt_app
        )
        #install(
        #    DIRECTORY ${CMAKE_INSTALL_PREFIX}/modules/
        #    DESTINATION Inviwo.app/Contents/Resources/modules
        #    COMPONENT qt_app
        #)
        install(
            DIRECTORY ${IVW_ROOT_DIR}/data/ 
            DESTINATION Inviwo.app/Contents/Resources/data
            COMPONENT qt_app
        )

        set(CPACK_BINARY_DRAGNDROP ON)

    else()
        install(TARGETS inviwo
                RUNTIME DESTINATION bin
                BUNDLE DESTINATION bin
                ARCHIVE DESTINATION lib
                LIBRARY DESTINATION lib
                COMPONENT qt_app)
    endif()
endif()

#--------------------------------------------------------------------
# Add dependencies
ivw_add_dependencies(${package_list})

#--------------------------------------------------------------------
# Optimize compilation with pre-compilied headers based on inviwo-core
ivw_compile_optimize_inviwo_core()

ivw_memleak_setup(inviwo)
