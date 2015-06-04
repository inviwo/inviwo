#--------------------------------------------------------------------
# Inviwo Qt Widgets
ivw_project(inviwo-qtwidgets)

#--------------------------------------------------------------------
# Add MOC files
set(MOC_FILES
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/angleradiuswidget.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/colorwheel.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/customdoublespinboxqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/editablelabelqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/sliderwidgetqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/inviwoapplicationqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/inviwodockwidget.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/inviwodockwidgettitlebar.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/lightpositionwidgetqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/ordinaleditorwidget.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/propertylistwidget.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/rangesliderqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/spinboxrangesliderwidgetqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/timerqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/processors/processorwidgetqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/properties/anglepropertywidgetqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/properties/boolpropertywidgetqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/properties/boolcompositepropertywidgetqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/properties/buttonpropertywidgetqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/properties/collapsiblegroupboxwidgetqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/properties/colorpropertywidgetqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/properties/compositepropertywidgetqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/properties/directorypropertywidgetqt.h       
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/properties/eventpropertywidgetqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/properties/filepropertywidgetqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/properties/ordinalpropertywidgetqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/properties/htmleditorwidgetqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/properties/htmllistwidgetqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/properties/imageeditorwidgetqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/properties/lightpropertywidgetqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/properties/optionpropertywidgetqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/properties/ordinalminmaxpropertywidgetqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/properties/ordinalminmaxtextpropertywidgetqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/properties/propertysettingswidgetqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/properties/propertywidgetqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/properties/stringpropertywidgetqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/properties/syntaxhighlighter.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/properties/texteditorwidgetqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/properties/transferfunctioneditor.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/properties/transferfunctioneditorview.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/properties/transferfunctionpropertywidgetqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/properties/transferfunctionpropertydialog.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/propertyselectiontreewidget.h
)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/inviwoqtutils.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/eventconverterqt.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/inviwofiledialog.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/inviwoqtwidgetsdefine.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/labelgraphicsitem.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/properties/transferfunctioncontrolpointconnection.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/properties/transferfunctioneditorcontrolpoint.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/qtwidgetmodule.h
    ${IVW_INCLUDE_DIR}/inviwo/qt/widgets/rawdatareaderdialogqt.h
    ${MOC_FILES}
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    angleradiuswidget.cpp
    colorwheel.cpp
    customdoublespinboxqt.cpp
    editablelabelqt.cpp
    eventconverterqt.cpp
    inviwoapplicationqt.cpp
    inviwodockwidget.cpp
    inviwodockwidgettitlebar.cpp
    inviwoqtutils.cpp
    inviwofiledialog.cpp
    labelgraphicsitem.cpp
    lightpositionwidgetqt.cpp
    ordinaleditorwidget.cpp
    processors/processorwidgetqt.cpp
    properties/anglepropertywidgetqt.cpp
    properties/boolpropertywidgetqt.cpp
    properties/boolcompositepropertywidgetqt.cpp
    properties/buttonpropertywidgetqt.cpp  
    properties/collapsiblegroupboxwidgetqt.cpp
    properties/colorpropertywidgetqt.cpp
    properties/compositepropertywidgetqt.cpp
    properties/directorypropertywidgetqt.cpp    
    properties/eventpropertywidgetqt.cpp
    properties/filepropertywidgetqt.cpp
    properties/glslsyntaxhighlighter.cpp
    properties/htmleditorwidgetqt.cpp
    properties/htmllistwidgetqt.cpp
    properties/imageeditorwidgetqt.cpp
    properties/lightpropertywidgetqt.cpp
    properties/optionpropertywidgetqt.cpp
    properties/ordinalminmaxpropertywidgetqt.cpp
    properties/ordinalminmaxtextpropertywidgetqt.cpp
    properties/ordinalpropertywidgetqt.cpp
    properties/propertysettingswidgetqt.cpp
    properties/propertywidgetqt.cpp
    properties/pythonsyntaxhighlighter.cpp
    properties/stringpropertywidgetqt.cpp
    properties/syntaxhighlighter.cpp
    properties/texteditorwidgetqt.cpp
    properties/transferfunctioncontrolpointconnection.cpp
    properties/transferfunctioneditor.cpp
    properties/transferfunctioneditorcontrolpoint.cpp
    properties/transferfunctioneditorview.cpp
    properties/transferfunctionpropertydialog.cpp
    properties/transferfunctionpropertywidgetqt.cpp 
    propertylistwidget.cpp
    propertyselectiontreewidget.cpp
    qtwidgetmodule.cpp
    rangesliderqt.cpp
    rawdatareaderdialogqt.cpp
    sliderwidgetqt.cpp
    spinboxrangesliderwidgetqt.cpp
)

ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Define defintions
ivw_define_standard_definitions(IVW_QTWIDGETS)
ivw_define_qt_definitions()

#--------------------------------------------------------------------
# Create library
find_package(Qt5Core QUIET REQUIRED)
ivw_qt_wrap_cpp(MOC_FILES ${MOC_FILES})
source_group("MOC Files" FILES ${MOC_FILES})
add_library(inviwo-qtwidgets ${SOURCE_FILES} ${MOC_FILES} ${HEADER_FILES})

# Add qt dependencies to this list
if(DESIRED_QT_VERSION MATCHES 5)
     set(IVW_WIDGETS_QT_DEPENDS
        Qt5Core
        Qt5Gui
        Qt5Widgets
        Qt5Help
    )

    foreach(qtarget ${IVW_WIDGETS_QT_DEPENDS})
        ivw_add_dependencies(${qtarget})  
        ivw_qt_add_dependencies(${qtarget} qt_widgets)
    endforeach()

else()
    ivw_add_dependencies(Qt)
    set(QT_DONT_USE_QTCORE FALSE)
    set(QT_DONT_USE_QTGUI FALSE)
    set(QT_DONT_USE_QTHELP FALSE)
    set(QT_USE_QT3SUPPORT FALSE)
    set(QT_USE_QTASSISTANT FALSE)
    set(QT_USE_QTDESIGNER FALSE)
    set(QT_USE_QTMOTIF FALSE)
    set(QT_USE_QTMAIN FALSE)
    set(QT_USE_QTNETWORK FALSE)
    set(QT_USE_QTNSPLUGIN FALSE)
    set(QT_USE_QTOPENGL FALSE)
    set(QT_USE_QTSQL FALSE)
    set(QT_USE_QTXML FALSE)
    set(QT_USE_QTSVG FALSE)
    set(QT_USE_QTTEST FALSE)
    set(QT_USE_QTUITOOLS FALSE)
    set(QT_USE_QTDBUS FALSE)
    set(QT_USE_QTSCRIPT FALSE)
    set(QT_USE_QTASSISTANTCLIENT FALSE)
    set(QT_USE_QTWEBKIT FALSE)
    set(QT_USE_QTXMLPATTERNS FALSE)
    set(QT_USE_PHONON FALSE)
    
    if(IVW_PACKAGE_PROJECT)
        if(WIN32)
            install(FILES ${QT_BINARY_DIR}/QtCore${CMAKE_DEBUG_POSTFIX}4.dll DESTINATION bin COMPONENT qt_widgets CONFIGURATIONS Debug)
            install(FILES ${QT_BINARY_DIR}/QtCore4.dll DESTINATION bin COMPONENT qt_widgets CONFIGURATIONS Release)
            install(FILES ${QT_BINARY_DIR}/QtGui${CMAKE_DEBUG_POSTFIX}4.dll DESTINATION bin COMPONENT qt_widgets CONFIGURATIONS Debug)
            install(FILES ${QT_BINARY_DIR}/QtGui4.dll DESTINATION bin COMPONENT qt_widgets CONFIGURATIONS Release)
        endif()
    endif()
endif()

# Add core dependencies to this list
ivw_add_dependencies(InviwoCore)

#--------------------------------------------------------------------
# Optimize compilation with pre-compilied headers based on inviwo-core
ivw_compile_optimize_inviwo_core()

#--------------------------------------------------------------------
# Make package (for other projects to find)
ivw_set_cpack_name(qt_widgets)
ivw_make_package(InviwoQtWidgets inviwo-qtwidgets)
