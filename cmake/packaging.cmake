 #################################################################################
 #
 # Inviwo - Interactive Visualization Workshop
 #
 # Copyright (c) 2014-2015 Inviwo Foundation
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
set(CPACK_PACKAGE_NAME "Inviwo")
set(CPACK_PACKAGE_VENDOR "Inviwo Foundation")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Interactive Visualization Workshop")
set(CPACK_PACKAGE_VERSION_MAJOR "${IVW_MAJOR_VERSION}")
set(CPACK_PACKAGE_VERSION_MINOR "${IVW_MINOR_VERSION}")
set(CPACK_PACKAGE_VERSION_PATCH "${IVW_PATCH_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${IVW_ROOT_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${IVW_ROOT_DIR}/LICENSE")
if(WIN32)
    set(CPACK_PACKAGE_ICON "${IVW_ROOT_DIR}\\\\resources\\\\icons\\\\inviwo_light.png")
if(CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_v${IVW_VERSION}_64bit")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "Inviwo\\\\${IVW_VERSION}\\\\64bit")
    set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_NAME} ${IVW_VERSION} 64-bit")
else()
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_v${IVW_VERSION}_32bit")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "Inviwo\\\\${IVW_VERSION}\\\\32bit")
    set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_NAME} ${IVW_VERSION} 32-bit")
endif()
else()
set(CPACK_PACKAGE_ICON "${IVW_ROOT_DIR}/resources/icons/inviwo_light.png")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_v${IVW_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Inviwo/${IVW_VERSION}")
endif()

set(IVW_PACKAGES "core")
set(IVW_EXECUTABLES "")

set(CPACK_COMPONENT_CORE_DISPLAY_NAME "Core")
set(CPACK_COMPONENT_CORE_REQUIRED TRUE)
set(CPACK_COMPONENT_CORE_GROUP "Base")
set(CPACK_COMPONENT_GROUP_BASE_DESCRIPTION "Inviwo core and common shared modules")

list(APPEND IVW_PACKAGES "modules")
set(CPACK_COMPONENT_MODULES_DISPLAY_NAME "Shared Modules")
set(CPACK_COMPONENT_MODULES_DEPENDS core)
set(CPACK_COMPONENT_MODULES_GROUP "Base")

if(IVW_QT_WIDGETS)
list(APPEND IVW_PACKAGES "qt_widgets")
set(CPACK_COMPONENT_QT_WIDGETS_DISPLAY_NAME "Qt Widgets")
set(CPACK_COMPONENT_QT_WIDGETS_DEPENDS ${IVW_PACKAGES})
set(CPACK_COMPONENT_QT_WIDGETS_GROUP "Qt")
if(DESIRED_QT_VERSION MATCHES 5)
exec_program(${QT_QMAKE_EXECUTABLE} ARGS "-query QT_VERSION" OUTPUT_VARIABLE QTVERSION)
endif()
set(CPACK_COMPONENT_GROUP_QT_DESCRIPTION "Components dependent on Qt ${QTVERSION}")
set(CPACK_COMPONENT_GROUP_QT_DISPLAY_NAME "Qt Dependent")
endif()

if(IVW_QT_EDITOR)
list(APPEND IVW_PACKAGES "qt_editor")
set(CPACK_COMPONENT_QT_EDITOR_DISPLAY_NAME "Qt Editor")
set(CPACK_COMPONENT_QT_EDITOR_DEPENDS ${IVW_PACKAGES})
set(CPACK_COMPONENT_QT_EDITOR_GROUP "Qt")

list(APPEND IVW_PACKAGES "qt_modules")
set(CPACK_COMPONENT_QT_MODULES_DISPLAY_NAME "Qt Modules")
set(CPACK_COMPONENT_QT_MODULES_DEPENDS ${IVW_PACKAGES})
set(CPACK_COMPONENT_QT_MODULES_GROUP "Qt")
endif()

if(IVW_QT_APPLICATION)
list(APPEND IVW_PACKAGES "qt_app")
list(APPEND IVW_EXECUTABLES "inviwo;Inviwo")
set(CPACK_COMPONENT_QT_APP_DISPLAY_NAME "Qt Application")
set(CPACK_COMPONENT_QT_APP_DEPENDS ${IVW_PACKAGES})
set(CPACK_COMPONENT_QT_APP_GROUP "Qt")
endif()

if(IVW_MODULE_GLFW)
list(APPEND IVW_PACKAGES "glfw_modules")
set(CPACK_COMPONENT_GLFW_MODULES_DISPLAY_NAME "GLFW Modules")
set(CPACK_COMPONENT_GLFW_MODULES_DISABLED TRUE)
set(CPACK_COMPONENT_GLFW_MODULES_DEPENDS "core;modules")
set(CPACK_COMPONENT_GLFW_MODULES_GROUP "Xtra")
set(CPACK_COMPONENT_GROUP_XTRA_DESCRIPTION "Minimal applications based on GLFW or GLUT")
endif()

if(IVW_TINY_GLFW_APPLICATION)
list(APPEND IVW_PACKAGES "glfw_app")
#list(APPEND IVW_EXECUTABLES "glfwminimum;GLFWMinimum")
set(CPACK_COMPONENT_GLFW_APP_DISPLAY_NAME "GLFW Minimal Application")
set(CPACK_COMPONENT_GLFW_APP_DISABLED TRUE)
set(CPACK_COMPONENT_GLFW_APP_DEPENDS "core;modules;glfw_modules")
set(CPACK_COMPONENT_GLFW_APP_GROUP "Xtra")
endif()

list(APPEND IVW_PACKAGES "workspaces")
set(CPACK_COMPONENT_WORKSPACES_DISPLAY_NAME "Workspaces")
set(CPACK_COMPONENT_WORKSPACES_DEPENDS "volumes;images")
set(CPACK_COMPONENT_WORKSPACES_GROUP "Examples")
set(CPACK_COMPONENT_GROUP_EXAMPLES_DESCRIPTION "Scenes, data and scripts")

list(APPEND IVW_PACKAGES "volumes")
set(CPACK_COMPONENT_VOLUMES_DISPLAY_NAME "Volumes")
set(CPACK_COMPONENT_VOLUMES_GROUP "Examples")

list(APPEND IVW_PACKAGES "images")
set(CPACK_COMPONENT_IMAGES_DISPLAY_NAME "Images")
set(CPACK_COMPONENT_IMAGES_GROUP "Examples")

list(APPEND IVW_PACKAGES "scripts")
set(CPACK_COMPONENT_SCRIPTS_DISPLAY_NAME "Scripts")
set(CPACK_COMPONENT_SCRIPTS_GROUP "Examples")

set(CPACK_COMPONENTS_ALL ${IVW_PACKAGES})
set(CPACK_PACKAGE_EXECUTABLES ${IVW_EXECUTABLES})

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
set(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.inviwo.org")

# Help URL
set(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.inviwo.org")

else()
set(CPACK_GENERATOR "ZIP")
endif()
set(CPACK_NSIS_INSTALLED_ICON_NAME "bin/inviwo.exe")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
if(IVW_PACKAGE_INSTALLER)
#http://www.cmake.org/cmake/help/v3.2/module/CPackBundle.html
set(CPACK_GENERATOR "TGZ;DragNDrop")
set(CPACK_BUNDLE_NAME "Inviwo")
set(CPACK_BUNDLE_ICON "${IVW_ROOT_DIR}/resources/icons/inviwo_light.icns")
set(CPACK_BUNDLE_PLIST "${CMAKE_BINARY_DIR}/packaging/macosx/Info.plist")
set(CPACK_OSX_PACKAGE_VERSION 10.8)
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

install(FILES ${IVW_ROOT_DIR}/data/help/inviwo.qch DESTINATION data/help COMPONENT qt_editor)
install(FILES ${IVW_ROOT_DIR}/data/help/inviwo.qhc DESTINATION data/help COMPONENT qt_editor)
install(FILES ${IVW_ROOT_DIR}/data/help/inviwo.qhcp DESTINATION data/help COMPONENT qt_editor)
install(DIRECTORY ${IVW_ROOT_DIR}/data/images DESTINATION data COMPONENT images)
install(DIRECTORY ${IVW_ROOT_DIR}/data/scripts DESTINATION data COMPONENT scripts)
install(DIRECTORY ${IVW_ROOT_DIR}/data/volumes DESTINATION data COMPONENT volumes)
install(DIRECTORY ${IVW_ROOT_DIR}/data/workspaces DESTINATION data COMPONENT workspaces)
include(CPack)
endif()

