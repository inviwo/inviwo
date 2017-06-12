 #################################################################################
 #
 # Inviwo - Interactive Visualization Workshop
 #
 # Copyright (c) 2014-2017 Inviwo Foundation
 # All rights reserved.
 # 
 # Redistribution and use in source and binary forms, with or without
 # modification, are permitted provided that the following conditions are met: 
 # 
 # 1. Redistributions of source code must retain the above copyright notice, this
 # list of conditions and the following disclaimer. 
 # 2. Redistributions in binary form must reproduce the above copyright notice,
 # this list of conditions and the following disclaimer in the documentation
 # and/or other materials provided with the distribution. 
 # 
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 # ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 # WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 # DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 # ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 # (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 # LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 # ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 # (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 # SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 # 
 #################################################################################
 
if(IVW_PACKAGE_PROJECT)
    # Don't use components, will not work with Drag & Drop, and we need controll all the 
    # component namnes which we can't for some of the externals like HDF5
    set(CPACK_MONOLITHIC_INSTALL TRUE)
    set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
    if(OPENMP_ON)
        set(CMAKE_INSTALL_OPENMP_LIBRARIES TRUE)
    endif()
    include (InstallRequiredSystemLibraries)

    set(CPACK_PACKAGE_NAME                "Inviwo")
    set(CPACK_PACKAGE_VENDOR              "Inviwo Foundation")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Interactive Visualization Workshop")
    set(CPACK_PACKAGE_VERSION_MAJOR       "${IVW_MAJOR_VERSION}")
    set(CPACK_PACKAGE_VERSION_MINOR       "${IVW_MINOR_VERSION}")
    set(CPACK_PACKAGE_VERSION_PATCH       "${IVW_PATCH_VERSION}")
    set(CPACK_PACKAGE_DESCRIPTION_FILE    "${IVW_ROOT_DIR}/README.md")
    set(CPACK_RESOURCE_FILE_LICENSE       "${IVW_ROOT_DIR}/LICENSE")

    if(WIN32)
        set(CPACK_PACKAGE_ICON "${IVW_ROOT_DIR}\\\\resources\\\\icons\\\\inviwo_light.png")
        # Need backslash for correct subdirectory paths with NSIS
        if(CMAKE_SIZEOF_VOID_P EQUAL 8 )
            set(CPACK_PACKAGE_FILE_NAME         "${CPACK_PACKAGE_NAME}-v${IVW_VERSION}-64bit")
            set(CPACK_PACKAGE_INSTALL_DIRECTORY "Inviwo\\\\${IVW_VERSION}\\\\64bit")
            set(CPACK_NSIS_DISPLAY_NAME         "${CPACK_PACKAGE_NAME} ${IVW_VERSION} 64-bit")
        else()
            set(CPACK_PACKAGE_FILE_NAME         "${CPACK_PACKAGE_NAME}-v${IVW_VERSION}-32bit")
            set(CPACK_PACKAGE_INSTALL_DIRECTORY "Inviwo\\\\${IVW_VERSION}\\\\32bit")
            set(CPACK_NSIS_DISPLAY_NAME         "${CPACK_PACKAGE_NAME} ${IVW_VERSION} 32-bit")
        endif()
    else()
        set(CPACK_PACKAGE_ICON              "${IVW_ROOT_DIR}/resources/icons/inviwo_light.png")
        set(CPACK_PACKAGE_FILE_NAME         "${CPACK_PACKAGE_NAME}-v${IVW_VERSION}")
        set(CPACK_PACKAGE_INSTALL_DIRECTORY "Inviwo/${IVW_VERSION}")
    endif()

    set(CPACK_PACKAGE_EXECUTABLES "inviwo;Inviwo") 
    option(IVW_PACKAGE_INSTALLER "Use NSIS to create installer" OFF)
    
    if(WIN32)
        if(IVW_PACKAGE_INSTALLER)
            set(CPACK_GENERATOR "ZIP;NSIS")
            # Create the desktop link
            set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "CreateShortCut '$DESKTOP\\\\${CPACK_NSIS_DISPLAY_NAME}.lnk' '$INSTDIR\\\\bin\\\\inviwo.exe' ")
            # Delete the desktop link
            set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "Delete '$DESKTOP\\\\${CPACK_NSIS_DISPLAY_NAME}.lnk' ")
            # The icon to start the application.
            set(CPACK_NSIS_MUI_ICON "${IVW_ROOT_DIR}\\\\resources\\\\icons\\\\inviwo_light.ico")
            # Add a link to the application website in the startup menu.
            set(CPACK_NSIS_MENU_LINKS "http://www.inviwo.org" "Inviwo Homepage")
            # Set the icon for the application in the Add/Remove programs section.
            set(CPACK_NSIS_INSTALLED_ICON_NAME bin\\\\inviwo.exe)
            # The mail address for the maintainer of the application in the Add/Remove programs section
            set(CPACK_NSIS_CONTACT info at inviwo.org)
            # The url of the application in the Add/Remove programs section
            set(CPACK_NSIS_URL_INFO_ABOUT "http://www.inviwo.org")
            # Help URL
            set(CPACK_NSIS_HELP_LINK "http://www.inviwo.org")

        else()
            set(CPACK_GENERATOR "ZIP")
        endif()

    elseif(APPLE)
        if(IVW_PACKAGE_INSTALLER)
            configure_file(${IVW_CMAKE_TEMPLATES}/info_plist_template.txt
               ${CMAKE_BINARY_DIR}/Info.plist
               @ONLY)
            #http://www.cmake.org/cmake/help/v3.2/module/CPackBundle.html
            set(CPACK_GENERATOR           "TGZ;DragNDrop")
            set(CPACK_BUNDLE_NAME         "Inviwo")
            set(CPACK_BUNDLE_ICON         "${IVW_ROOT_DIR}/Resources/icons/inviwo_light.icns")
            set(CPACK_BUNDLE_PLIST        "${CMAKE_BINARY_DIR}/Info.plist")
            set(CPACK_DMG_DS_STORE        "${IVW_ROOT_DIR}/Resources/DS_mapp")
            set(CPACK_DMG_VOLUME_NAME     "Inviwo ${IVW_VERSION}")
            set(CPACK_OSX_PACKAGE_VERSION 10.10)
        else()
            set(CPACK_GENERATOR "TGZ")
        endif()

    else()
        if(IVW_PACKAGE_INSTALLER)
            set(CPACK_GENERATOR "TGZ;DEB")
        else()
            set(CPACK_GENERATOR "TGZ")
        endif()
    endif()

    if(NOT APPLE)
        install(DIRECTORY ${IVW_ROOT_DIR}/data/images           DESTINATION data      COMPONENT images)
        install(DIRECTORY ${IVW_ROOT_DIR}/data/scripts          DESTINATION data      COMPONENT scripts)
        install(DIRECTORY ${IVW_ROOT_DIR}/data/volumes          DESTINATION data      COMPONENT volumes)
        install(DIRECTORY ${IVW_ROOT_DIR}/data/workspaces       DESTINATION data      COMPONENT workspaces)
    endif()

    include(CPack)
endif()

