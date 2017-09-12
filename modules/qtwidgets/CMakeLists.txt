#--------------------------------------------------------------------
# Inviwo Qt Widgets
ivw_module(QtWidgets)

find_package(Qt5Core QUIET REQUIRED)
find_package(Qt5Gui QUIET REQUIRED)
find_package(Qt5Widgets QUIET REQUIRED)

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
    properties/propertywidgetqt.h
    properties/stringmultilinepropertywidgetqt.h
    properties/texteditorwidgetqt.h
    properties/transferfunctioneditor.h
    properties/transferfunctionpropertydialog.h
    propertylistwidget.h
    rangesliderqt.h
    sliderwidgetqt.h
)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${MOC_FILES}
    eventconverterqt.h
    inviwoqtutils.h
    keyboardutils.h
    labelgraphicsitem.h
    properties/anglepropertywidgetqt.h
    properties/boolpropertywidgetqt.h
    properties/boolcompositepropertywidgetqt.h
    properties/buttonpropertywidgetqt.h
    properties/collapsiblegroupboxwidgetqt.h
    properties/colorpropertywidgetqt.h
    properties/compositepropertywidgetqt.h
    properties/eventpropertywidgetqt.h
    properties/filepropertywidgetqt.h
    properties/lightpropertywidgetqt.h
    properties/multifilepropertywidgetqt.h
    properties/optionpropertywidgetqt.h
    properties/ordinalminmaxpropertywidgetqt.h
    properties/ordinalminmaxtextpropertywidgetqt.h
    properties/ordinalpropertywidgetqt.h
    properties/propertyeditorwidgetqt.h
    properties/propertysettingswidgetqt.h
    properties/stringpropertywidgetqt.h
    properties/syntaxhighlighter.h
    properties/transferfunctioncontrolpointconnection.h
    properties/transferfunctioneditorcontrolpoint.h
    properties/transferfunctioneditorview.h
    properties/transferfunctionpropertywidgetqt.h
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
    qtwidgetsmodule.cpp
    rangesliderqt.cpp
    rawdatareaderdialogqt.cpp
    sliderwidgetqt.cpp
)

ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Create library
ivw_qt_wrap_cpp(MOC_FILES ${MOC_FILES})
source_group("MOC Files" FILES ${MOC_FILES})

#--------------------------------------------------------------------
# Define defintions
ivw_define_qt_definitions()

#--------------------------------------------------------------------
# Create module
ivw_set_cpack_name(qt_widgets)
ivw_create_module(${SOURCE_FILES} ${MOC_FILES} ${HEADER_FILES})
target_link_libraries(inviwo-module-qtwidgets PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets)
ivw_make_package(InviwoQtWidgetsModule inviwo-module-qtwidgets)

ivw_qt_add_to_install(qt_widgets Qt5Core Qt5Gui Qt5Widgets)

