#--------------------------------------------------------------------
# Inviwo Qt Widgets
ivw_module(QtWidgets)

#--------------------------------------------------------------------
# Add MOC files
set(MOC_FILES
    angleradiuswidget.h
    colorwheel.h
    customdoublespinboxqt.h
    editablelabelqt.h
    filepathlineeditqt.h
    inviwodockwidget.h
    inviwodockwidgettitlebar.h
    inviwofiledialog.h
    inviwowidgetsqt.h
    lightpositionwidgetqt.h
    lineeditqt.h
    ordinaleditorwidget.h
    processors/processordockwidgetqt.h
    processors/processorwidgetqt.h
    properties/anglepropertywidgetqt.h
    properties/boolcompositepropertywidgetqt.h
    properties/buttonpropertywidgetqt.h
    properties/collapsiblegroupboxwidgetqt.h
    properties/colorpropertywidgetqt.h
    properties/compositepropertywidgetqt.h
    properties/eventpropertywidgetqt.h
    properties/filepropertywidgetqt.h
    properties/htmleditorwidgetqt.h
    properties/htmllistwidgetqt.h
    properties/imageeditorwidgetqt.h
    properties/lightpropertywidgetqt.h
    properties/multifilepropertywidgetqt.h
    properties/optionpropertywidgetqt.h
    properties/ordinalminmaxpropertywidgetqt.h
    properties/ordinalminmaxtextpropertywidgetqt.h
    properties/ordinalpropertywidgetqt.h
    properties/propertyeditorwidgetqt.h
    properties/propertysettingswidgetqt.h
    properties/propertywidgetqt.h
    properties/stringmultilinepropertywidgetqt.h
    properties/stringpropertywidgetqt.h
    properties/syntaxhighlighter.h
    properties/texteditorwidgetqt.h
    properties/transferfunctioneditor.h
    properties/transferfunctioneditorview.h
    properties/transferfunctionpropertydialog.h
    properties/transferfunctionpropertywidgetqt.h
    propertylistwidget.h
    propertyselectiontreewidget.h
    rangesliderqt.h
    sliderwidgetqt.h
    spinboxrangesliderwidgetqt.h
)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${MOC_FILES}
    eventconverterqt.h
    inviwoqtutils.h
    keyboardutils.h
    labelgraphicsitem.h
    properties/boolpropertywidgetqt.h
    properties/transferfunctioncontrolpointconnection.h
    properties/transferfunctioneditorcontrolpoint.h
    qstringhelper.h
    qtwidgetsmodule.h
    qtwidgetsmoduledefine.h
    rawdatareaderdialogqt.h
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
    filepathlineeditqt.cpp
    inviwodockwidget.cpp
    inviwodockwidgettitlebar.cpp
    inviwofiledialog.cpp
    inviwoqtutils.cpp
    inviwowidgetsqt.cpp
    keyboardutils.cpp
    labelgraphicsitem.cpp
    lightpositionwidgetqt.cpp
    lineeditqt.cpp
    ordinaleditorwidget.cpp
    processors/processordockwidgetqt.cpp
    processors/processorwidgetqt.cpp
    properties/anglepropertywidgetqt.cpp
    properties/boolcompositepropertywidgetqt.cpp
    properties/boolpropertywidgetqt.cpp
    properties/buttonpropertywidgetqt.cpp  
    properties/collapsiblegroupboxwidgetqt.cpp
    properties/colorpropertywidgetqt.cpp
    properties/compositepropertywidgetqt.cpp
    properties/eventpropertywidgetqt.cpp
    properties/filepropertywidgetqt.cpp
    properties/glslsyntaxhighlighter.cpp
    properties/htmleditorwidgetqt.cpp
    properties/htmllistwidgetqt.cpp
    properties/imageeditorwidgetqt.cpp
    properties/lightpropertywidgetqt.cpp
    properties/multifilepropertywidgetqt.cpp
    properties/optionpropertywidgetqt.cpp
    properties/ordinalminmaxpropertywidgetqt.cpp
    properties/ordinalminmaxtextpropertywidgetqt.cpp
    properties/ordinalpropertywidgetqt.cpp
    properties/propertyeditorwidgetqt.cpp
    properties/propertysettingswidgetqt.cpp
    properties/propertywidgetqt.cpp
    properties/pythonsyntaxhighlighter.cpp
    properties/stringmultilinepropertywidgetqt.cpp
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
    qtwidgetsmodule.cpp
    rangesliderqt.cpp
    rawdatareaderdialogqt.cpp
    sliderwidgetqt.cpp
    spinboxrangesliderwidgetqt.cpp
)

ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Create library
find_package(Qt5Core QUIET REQUIRED)
ivw_qt_wrap_cpp(MOC_FILES ${MOC_FILES})
source_group("MOC Files" FILES ${MOC_FILES})
#add_library(inviwo-qtwidgets ${SOURCE_FILES} ${MOC_FILES} ${HEADER_FILES})


#--------------------------------------------------------------------
# Define defintions
#ivw_define_standard_definitions(IVW_QTWIDGETS inviwo-qtwidgets)
#ivw_define_standard_properties(inviwo-qtwidgets)
ivw_define_qt_definitions()



# Add core dependencies to this list
#ivw_add_dependencies(InviwoCore)


#--------------------------------------------------------------------
# Make package (for other projects to find)
#ivw_set_cpack_name(qt_widgets)
#ivw_make_package(InviwoQtWidgets inviwo-qtwidgets)

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${MOC_FILES} ${HEADER_FILES})

# Add qt dependencies to this list
set(IVW_WIDGETS_QT_DEPENDS
    Qt5Core
    Qt5Gui
    Qt5Widgets
    Qt5Help
)

foreach(qtarget ${IVW_WIDGETS_QT_DEPENDS})
    #ivw_add_dependencies(${qtarget})
    ivw_qt_add_to_install(${qtarget} qt_widgets)
endforeach()
