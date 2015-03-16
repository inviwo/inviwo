#--------------------------------------------------------------------
# Inviwo Qt Editor
ivw_project(inviwo-qteditor)

#--------------------------------------------------------------------
# Add MOC files
set(MOC_FILES
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/consolewidget.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/networkeditor.h
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
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/networkeditorview.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/processorgraphicsitem.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/processorlinkgraphicsitem.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/processorportgraphicsitem.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/processorprogressgraphicsitem.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/editor/processorstatusgraphicsitem.h
    ${MOC_FILES}
)
ivw_group("Header Files" ${HEADER_FILES})

ivw_qt_wrap_cpp(MOC_FILES ${MOC_FILES})
source_group("MOC Files" FILES ${MOC_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    connectiongraphicsitem.cpp
    consolewidget.cpp
    helpwidget.cpp
    editorgrapicsitem.cpp
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
add_library(inviwo-qteditor ${SOURCE_FILES} ${MOC_FILES} ${HEADER_FILES})

# Add dependencies to this list
if(DESIRED_QT_VERSION MATCHES 5)
    ivw_add_dependencies(InviwoQtWidgets Qt5Help)
    
    if(IVW_PACKAGE_PROJECT)
        if(WIN32)
            if(Qt5Help_FOUND)
                set(QT_BINARY_DIR "${Qt5Help_DIR}/../../../bin")
                install(FILES ${QT_BINARY_DIR}/Qt5Help${CMAKE_DEBUG_POSTFIX}.dll DESTINATION bin COMPONENT qt_modules CONFIGURATIONS Debug)
                install(FILES ${QT_BINARY_DIR}/Qt5Help.dll DESTINATION bin COMPONENT qt_modules CONFIGURATIONS Release)
                install(FILES ${QT_BINARY_DIR}/Qt5CLucene${CMAKE_DEBUG_POSTFIX}.dll DESTINATION bin COMPONENT qt_modules CONFIGURATIONS Debug)
                install(FILES ${QT_BINARY_DIR}/Qt5CLucene.dll DESTINATION bin COMPONENT qt_modules CONFIGURATIONS Release)
                install(FILES ${QT_BINARY_DIR}/Qt5Sql${CMAKE_DEBUG_POSTFIX}.dll DESTINATION bin COMPONENT qt_modules CONFIGURATIONS Debug)
                install(FILES ${QT_BINARY_DIR}/Qt5Sql.dll DESTINATION bin COMPONENT qt_modules CONFIGURATIONS Release)
                install(FILES ${QT_BINARY_DIR}/Qt5Network${CMAKE_DEBUG_POSTFIX}.dll DESTINATION bin COMPONENT qt_modules CONFIGURATIONS Debug)
                install(FILES ${QT_BINARY_DIR}/Qt5Network.dll DESTINATION bin COMPONENT qt_modules CONFIGURATIONS Release)
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