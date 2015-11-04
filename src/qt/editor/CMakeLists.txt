#--------------------------------------------------------------------
# Inviwo Qt Editor
ivw_project(inviwo-qteditor)

#--------------------------------------------------------------------
# Add MOC files
set(MOC_FILES
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/consolewidget.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/networkeditor.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/inviwomainwindow.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/processorlistwidget.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/helpwidget.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/resourcemanagerwidget.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/settingswidget.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/linkdialog/linkdialog.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/linkdialog/linkdialogscene.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/linkdialog/linkdialogprocessorgraphicsitems.h
)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/connectiongraphicsitem.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/editorgrapicsitem.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/inviwoqteditordefine.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/linkdialog/linkdialogcurvegraphicsitems.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/linkdialog/linkdialoggraphicsitems.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/linkdialog/linkdialogpropertygraphicsitems.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/linkdialog/linkdialogview.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/linkgraphicsitem.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/networkeditorobserver.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/networkeditorview.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/processorgraphicsitem.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/processorlinkgraphicsitem.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/processorportgraphicsitem.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/processorpreview.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/processorprogressgraphicsitem.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/processorstatusgraphicsitem.h
    ${MOC_FILES}
)
ivw_group("Header Files" ${HEADER_FILES})



#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    connectiongraphicsitem.cpp
    consolewidget.cpp
    editorgrapicsitem.cpp
    helpwidget.cpp
    inviwomainwindow.cpp
    linkdialog/linkdialog.cpp
    linkdialog/linkdialogcurvegraphicsitems.cpp
    linkdialog/linkdialogprocessorgraphicsitems.cpp
    linkdialog/linkdialogpropertygraphicsitems.cpp
    linkdialog/linkdialogscene.cpp
    linkdialog/linkdialogview.cpp
    linkgraphicsitem.cpp
    networkeditor.cpp
    networkeditorview.cpp
    processorgraphicsitem.cpp
    processorlinkgraphicsitem.cpp
    processorlistwidget.cpp
    processorportgraphicsitem.cpp
    processorpreview.cpp
    processorprogressgraphicsitem.cpp
    processorstatusgraphicsitem.cpp
    resourcemanagerwidget.cpp
    settingswidget.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(IVW_QTEDITOR)
ivw_define_qt_definitions()

#--------------------------------------------------------------------
# Create library
if(DESIRED_QT_VERSION MATCHES 5)
    find_package(Qt5Core QUIET REQUIRED)
else()
    find_package(Qt QUIET REQUIRED)
endif()
ivw_qt_wrap_cpp(MOC_FILES ${MOC_FILES})
source_group("MOC Files" FILES ${MOC_FILES})
add_library(inviwo-qteditor ${SOURCE_FILES} ${MOC_FILES} ${HEADER_FILES})

# Add dependencies to this list
if(DESIRED_QT_VERSION MATCHES 5)
    set(IVW_EDITOR_QT_DEPENDS
        Qt5Core
        Qt5Gui
        Qt5Widgets
        Qt5Help
    )

    foreach(qtarget ${IVW_EDITOR_QT_DEPENDS})
        ivw_add_dependencies(${qtarget})  
        ivw_qt_add_to_install(${qtarget} qt_editor)
    endforeach()

    foreach(qtarget Qt5Sql Qt5Network)
        ivw_qt_add_to_install(${qtarget} qt_editor)
    endforeach()

    ivw_add_dependencies(InviwoQtWidgets )
    
    if(IVW_PACKAGE_PROJECT)
        if(WIN32)
            if(Qt5Help_FOUND)
                set(QT_BINARY_DIR "${Qt5Help_DIR}/../../../bin")
                install(FILES ${QT_BINARY_DIR}/Qt5Help${CMAKE_DEBUG_POSTFIX}.dll DESTINATION bin COMPONENT qt_modules CONFIGURATIONS Debug)
                install(FILES ${QT_BINARY_DIR}/Qt5Help.dll DESTINATION bin COMPONENT qt_modules CONFIGURATIONS Release)
            endif()
        endif()
    endif()
else()
    set(QT_USE_QTHELP TRUE)
    ivw_add_definition(QT_HELP_LIB)

    ivw_add_dependencies(InviwoQtWidgets Qt)
        
    if(IVW_PACKAGE_PROJECT)
        if(WIN32)
            install(FILES ${QT_BINARY_DIR}/QtHelp${CMAKE_DEBUG_POSTFIX}4.dll DESTINATION bin COMPONENT qt_modules CONFIGURATIONS Debug)
            install(FILES ${QT_BINARY_DIR}/QtHelp4.dll DESTINATION bin COMPONENT qt_modules CONFIGURATIONS Release)
        endif()
    endif()
endif()



#--------------------------------------------------------------------
# Optimize compilation with pre-compilied headers based on inviwo-core
ivw_compile_optimize_inviwo_core()

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_set_cpack_name(qt_editor)
ivw_make_package(InviwoQtEditor inviwo-qteditor)