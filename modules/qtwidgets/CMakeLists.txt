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
    properties/stringmultilinepropertywidgetqt.h
    properties/transferfunctioneditor.h
    propertylistwidget.h
    rangesliderqt.h
    sliderwidgetqt.h
)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/isovaluepropertywidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/transferfunctioneditorprimitive.h
    ${MOC_FILES}
    eventconverterqt.h
    inviwoqtutils.h
    keyboardutils.h
    labelgraphicsitem.h
    properties/anglepropertywidgetqt.h
    properties/boolcompositepropertywidgetqt.h
    properties/boolpropertywidgetqt.h
    properties/buttonpropertywidgetqt.h
    properties/collapsiblegroupboxwidgetqt.h
    properties/colorpropertywidgetqt.h
    properties/compositepropertywidgetqt.h
    properties/eventpropertywidgetqt.h
    properties/filepropertywidgetqt.h
    properties/fontsizepropertywidgetqt.h
    properties/lightpropertywidgetqt.h
    properties/multifilepropertywidgetqt.h
    properties/optionpropertywidgetqt.h
    properties/ordinalminmaxpropertywidgetqt.h
    properties/ordinalminmaxtextpropertywidgetqt.h
    properties/ordinalpropertywidgetqt.h
    properties/propertyeditorwidgetqt.h
    properties/propertysettingswidgetqt.h
    properties/propertywidgetqt.h
    properties/stringpropertywidgetqt.h
    properties/syntaxhighlighter.h
    properties/texteditorwidgetqt.h
    properties/transferfunctioncontrolpointconnection.h
    properties/transferfunctioneditorcontrolpoint.h
    properties/transferfunctioneditorview.h
    properties/transferfunctionpropertydialog.h
    properties/transferfunctionpropertywidgetqt.h
    qstringhelper.h
    qtwidgetsmodule.h
    qtwidgetsmoduledefine.h
    qtwidgetssettings.h
    rawdatareaderdialogqt.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/isovaluepropertywidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/transferfunctioneditorprimitive.cpp
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
    properties/fontsizepropertywidgetqt.cpp
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
    qtwidgetssettings.cpp
    rangesliderqt.cpp
    rawdatareaderdialogqt.cpp
    sliderwidgetqt.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Create library
qt5_wrap_cpp(MOCED_FILES ${MOC_FILES})
source_group("MOC Files" FILES ${MOCED_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${MOCED_FILES} ${HEADER_FILES})
target_link_libraries(inviwo-module-qtwidgets PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets)
ivw_make_package(InviwoQtWidgetsModule inviwo-module-qtwidgets)
ivw_qt_add_to_install(qt_widgets Qt5Core Qt5Gui Qt5Widgets)

ivw_register_license_file(NAME "Qt" VERSION ${Qt5Widgets_VERSION} MODULE QtWidgets TYPE "LGPLv3"
    URL https://www.qt.io/
    FILES ${IVW_EXTENSIONS_DIR}/qt/LICENSE.txt
)

