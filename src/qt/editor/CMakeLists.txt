#--------------------------------------------------------------------
# Inviwo Qt Editor
project(inviwo-qteditor)

#--------------------------------------------------------------------
# Add MOC files
set(MOC_FILES
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/connectiondraghelper.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/consolewidget.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/filetreemodel.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/filetreewidget.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/linkdialog/linkdialogscene.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/linkdraghelper.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/networkeditor.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/processordraghelper.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/processormimedata.h
)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/annotationswidget.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/connectiongraphicsitem.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/dataopener.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/editorgrapicsitem.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/fileassociations.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/helpwidget.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/inviwoaboutwindow.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/inviwoeditmenu.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/inviwomainwindow.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/inviwoqteditordefine.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/linkdialog/linkdialog.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/linkdialog/linkdialogcurvegraphicsitems.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/linkdialog/linkdialoggraphicsitems.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/linkdialog/linkdialogprocessorgraphicsitems.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/linkdialog/linkdialogpropertygraphicsitems.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/linkdialog/linkdialogview.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/linkgraphicsitem.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/networkeditorobserver.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/networkeditorview.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/networksearch.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/processorgraphicsitem.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/processorlinkgraphicsitem.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/processorlistwidget.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/processorportgraphicsitem.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/processorpreview.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/processorprogressgraphicsitem.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/processorstatusgraphicsitem.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/resourcemanager/resourcemanagerdockwidget.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/settingswidget.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/toolsmenu.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/undomanager.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/welcomewidget.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/workspaceannotationsqt.h
    ${MOC_FILES}
)
if(TARGET inviwo::meta)
    list(APPEND HEADER_FILES ${IVW_INCLUDE_DIR}/inviwo/qt/editor/toolsmetamenu.h)
endif()

ivw_group("Header Files" BASE ${IVW_INCLUDE_DIR}/inviwo/qt/editor/ ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    annotationswidget.cpp
    connectiondraghelper.cpp
    connectiongraphicsitem.cpp
    consolewidget.cpp
    dataopener.cpp
    editorgrapicsitem.cpp
    fileassociations.cpp
    filetreemodel.cpp
    filetreewidget.cpp
    helpwidget.cpp
    inviwoaboutwindow.cpp
    inviwoeditmenu.cpp
    inviwoeditmenu.cpp
    inviwomainwindow.cpp
    linkdialog/linkdialog.cpp
    linkdialog/linkdialogcurvegraphicsitems.cpp
    linkdialog/linkdialogprocessorgraphicsitems.cpp
    linkdialog/linkdialogpropertygraphicsitems.cpp
    linkdialog/linkdialogscene.cpp
    linkdialog/linkdialogview.cpp
    linkdraghelper.cpp
    linkgraphicsitem.cpp
    networkeditor.cpp
    networkeditorview.cpp
    networksearch.cpp
    processordraghelper.cpp
    processorgraphicsitem.cpp
    processorlinkgraphicsitem.cpp
    processorlistwidget.cpp
    processormimedata.cpp
    processorportgraphicsitem.cpp
    processorpreview.cpp
    processorprogressgraphicsitem.cpp
    processorstatusgraphicsitem.cpp
    resourcemanager/resourcemanagerdockwidget.cpp
    settingswidget.cpp
    toolsmenu.cpp
    undomanager.cpp
    welcomewidget.cpp
    workspaceannotationsqt.cpp
)
if(TARGET inviwo::meta)
    list(APPEND SOURCE_FILES toolsmetamenu.cpp)
endif()

ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Create library
find_package(Qt5Core 5.12 QUIET REQUIRED)
find_package(Qt5Gui QUIET REQUIRED)
find_package(Qt5Widgets QUIET REQUIRED)
find_package(Qt5Help QUIET REQUIRED)
find_package(Qt5Svg QUIET REQUIRED)

qt5_wrap_cpp(MOCED_FILES ${MOC_FILES})
source_group("MOC Files" FILES ${MOCED_FILES})
add_library(inviwo-qteditor ${SOURCE_FILES} ${MOCED_FILES} ${HEADER_FILES})
add_library(inviwo::qteditor ALIAS inviwo-qteditor)
target_link_libraries(inviwo-qteditor PUBLIC 
    inviwo::core 
    inviwo::qtapplicationbase
    inviwo::module::qtwidgets
    Qt5::Core 
    Qt5::Gui 
    Qt5::Widgets 
    Qt5::Help
    Qt5::Svg
)
if(TARGET inviwo::meta)
    target_link_libraries(inviwo-qteditor PUBLIC inviwo::meta)
    target_compile_definitions(inviwo-qteditor PRIVATE IVW_INVIWO_META)
endif()

#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(IVW_QTEDITOR inviwo-qteditor)
ivw_define_standard_properties(inviwo-qteditor)

#--------------------------------------------------------------------
# Optimize compilation with pre-compilied headers
ivw_compile_optimize_on_target(inviwo-qteditor)

#--------------------------------------------------------------------
# Generate html version of changelog (for the welcome widget), if python is available
find_package(PythonInterp QUIET)
if (PYTHONINTERP_FOUND)
    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/changelog.html
        COMMAND ${PYTHON_EXECUTABLE} ${IVW_ROOT_DIR}/tools/changelog-to-html.py -i ${IVW_ROOT_DIR}/CHANGELOG.md -o ${CMAKE_BINARY_DIR}/changelog.html
        WORKING_DIRECTORY ${IVW_ROOT_DIR}
        MAIN_DEPENDENCY ${IVW_ROOT_DIR}/CHANGELOG.md
        DEPENDS ${IVW_ROOT_DIR}/tools/changelog-to-html.py
        COMMENT "Generating HTML version of the Inviwo Changelog"
        VERBATIM
    )
    add_custom_target("inviwo-changelog"
        DEPENDS ${CMAKE_BINARY_DIR}/changelog.html
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_BINARY_DIR}/changelog.html
            ${IVW_RESOURCES_DIR}/changelog.html
    )

    add_dependencies("inviwo-qteditor" "inviwo-changelog")
endif()

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_default_install_comp_targets(qt_editor inviwo-qteditor)
ivw_qt_add_to_install(qt_editor Qt5Core Qt5Gui Qt5Widgets Qt5Help Qt5Sql Qt5Svg) # Qt5Sql is needed by Qt5Help
ivw_make_package(InviwoQtEditor inviwo-qteditor)

ivw_register_license_file(NAME "DocumentWindow" MODULE QtWidgets
    URL "https://wiki.qt.io/Assigning_a_file_type_to_an_Application_on_Windows"
    TYPE "Simplified BSD License"
    FILES ${IVW_INCLUDE_DIR}/inviwo/qt/editor/fileassociations.license.txt
)
