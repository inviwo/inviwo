#################################################################################
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2013-2020 Inviwo Foundation
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
set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
set(CMAKE_INSTALL_OPENMP_LIBRARIES TRUE)
set(CMAKE_INSTALL_SYSTEM_RUNTIME_COMPONENT Application)
include (InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME                "Inviwo Dome")
set(CPACK_PACKAGE_VENDOR              "Inviwo Foundation")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Interactive Visualization Workshop")
set(CPACK_PACKAGE_VERSION_MAJOR       "${IVW_MAJOR_VERSION}")
set(CPACK_PACKAGE_VERSION_MINOR       "${IVW_MINOR_VERSION}")
set(CPACK_PACKAGE_VERSION_PATCH       "${IVW_PATCH_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_FILE    "${IVW_ROOT_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE       "${IVW_ROOT_DIR}/LICENSE")
set(CPACK_PACKAGE_FILE_NAME           "${CPACK_PACKAGE_NAME}-v${IVW_VERSION}")

set(CPACK_MONOLITHIC_INSTALL OFF)
set(CPACK_NSIS_MANIFEST_DPI_AWARE ON)
ivw_get_target_property_recursive(install_list inviwo-dome INTERFACE_IVW_INSTALL_LIST OFF)
ivw_filter_install_list(LIST install_list REMOVE_COMPONENTS Development Testing)
list(TRANSFORM install_list REPLACE "\\|%\\|" ";")
set(CPACK_INSTALL_CMAKE_PROJECTS ${install_list})

if(WIN32)
    # Need backslash for correct subdirectory paths with NSIS
    set(CPACK_PACKAGE_ICON              "${IVW_ROOT_DIR}\\\\resources\\\\inviwo\\\\inviwo_light.png")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME}\\\\${IVW_VERSION}")
else()
    set(CPACK_PACKAGE_ICON              "${IVW_ROOT_DIR}/resources/inviwo/inviwo_light.png")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME}/${IVW_VERSION}")
endif()

set(CPACK_PACKAGE_EXECUTABLES "inviwo-dome" "Inviwo Dome")
set(CPACK_CREATE_DESKTOP_LINKS "inviwo-dome")
option(IVW_PACKAGE_INSTALLER "Use NSIS to create installer" OFF)

if(WIN32)
    if(IVW_PACKAGE_INSTALLER)
        set(CPACK_GENERATOR "ZIP;NSIS")
        # The icon to start the application.
        set(CPACK_NSIS_MUI_ICON "${IVW_ROOT_DIR}\\\\resources\\\\inviwo\\\\inviwo_light.ico")
        # Add a link to the application website in the startup menu.
        set(CPACK_NSIS_MENU_LINKS "http://www.inviwo.org" "Inviwo Homepage")
        # Set the icon for the application in the Add/Remove programs section.
        set(CPACK_NSIS_INSTALLED_ICON_NAME bin\\\\inviwodome.exe)
        # The mail address for the maintainer of the application in the Add/Remove programs section
        set(CPACK_NSIS_CONTACT info at inviwo.org)
        # The url of the application in the Add/Remove programs section
        set(CPACK_NSIS_URL_INFO_ABOUT "http://www.inviwo.org")
        # Help URL
        set(CPACK_NSIS_HELP_LINK "http://www.inviwo.org")
        set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_NAME} ${IVW_VERSION}")
        set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "CreateShortCut '$INSTDIR\\\\inviwo.lnk' '$INSTDIR\\\\bin\\\\inviwodome.exe'")
        set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
        set(CPACK_NSIS_MUI_FINISHPAGE_RUN "${CPACK_PACKAGE_NAME}")
        set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "Delete '$INSTDIR\\\\inviwo.lnk'")
    else()
        set(CPACK_GENERATOR "ZIP")
    endif()

elseif(APPLE)
    if(IVW_PACKAGE_INSTALLER)
        set(CPACK_GENERATOR           "TGZ;DragNDrop")
        set(CPACK_DMG_DS_STORE        "${IVW_ROOT_DIR}/Resources/DS_mapp")
        set(CPACK_DMG_VOLUME_NAME     "${CPACK_PACKAGE_FILE_NAME}")
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

install(DIRECTORY ${IVW_ROOT_DIR}/data/  DESTINATION ${IVW_RESOURCE_INSTALL_PREFIX}data  COMPONENT Datasets)

include(CPack)

