# Inviwo Qt Application
project(inviwo_qtminimum)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Add source files
set(SOURCE_FILES
    qtminimum.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

set(CMAKE_FILES
    CMakeLists.txt
)
ivw_group("CMake Files" ${CMAKE_FILES})

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
add_executable(inviwo_qtminimum MACOSX_BUNDLE WIN32 ${SOURCE_FILES} ${RES_FILES} ${CMAKE_FILES})
target_link_libraries(inviwo_qtminimum PUBLIC 
    inviwo::core
    inviwo::module-system
    inviwo::module::qtwidgets
    inviwo::qtapplicationbase
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

ivw_define_standard_definitions(inviwo_qtminimum inviwo_qtminimum) 
ivw_define_standard_properties(inviwo_qtminimum)

# Move to folder
ivw_folder(inviwo_qtminimum apps)
# Add application to pack
ivw_default_install_targets(inviwo_qtminimum)
