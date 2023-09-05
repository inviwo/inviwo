# Inviwo Qt Widgets
ivw_module(QtWidgets)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg)
 
# Add MOC files
set(MOC_FILES
    include/modules/qtwidgets/angleradiuswidget.h
    include/modules/qtwidgets/colorwheel.h
    include/modules/qtwidgets/customdoublespinboxqt.h
    include/modules/qtwidgets/editablelabelqt.h
    include/modules/qtwidgets/filepathlineeditqt.h
    include/modules/qtwidgets/inviwodockwidget.h
    include/modules/qtwidgets/inviwodockwidgettitlebar.h
    include/modules/qtwidgets/inviwofiledialog.h
    include/modules/qtwidgets/inviwowidgetsqt.h
    include/modules/qtwidgets/lightpositionwidgetqt.h
    include/modules/qtwidgets/lineeditqt.h
    include/modules/qtwidgets/ordinaleditorwidget.h
    include/modules/qtwidgets/processors/processordockwidgetqt.h    
    include/modules/qtwidgets/properties/colorlineedit.h
    include/modules/qtwidgets/properties/doublevaluedragspinbox.h
    include/modules/qtwidgets/properties/ordinalspinboxwidget.h
    include/modules/qtwidgets/properties/stringmultilinepropertywidgetqt.h
    include/modules/qtwidgets/properties/valuedragspinbox.h
    include/modules/qtwidgets/propertylistwidget.h
    include/modules/qtwidgets/rangesliderqt.h
    include/modules/qtwidgets/sliderwidgetqt.h
    include/modules/qtwidgets/syntaxhighlighter.h
    include/modules/qtwidgets/tf/tfcoloredit.h
    include/modules/qtwidgets/tf/tfeditor.h
    include/modules/qtwidgets/tf/tflineedit.h
    include/modules/qtwidgets/tf/tfselectionwatcher.h
    include/modules/qtwidgets/tfhelpwindow.h
)

# Add header files
set(HEADER_FILES
    ${MOC_FILES}
    include/modules/qtwidgets/codeedit.h
    include/modules/qtwidgets/editorfileobserver.h
    include/modules/qtwidgets/editorsettings.h
    include/modules/qtwidgets/eventconverterqt.h
    include/modules/qtwidgets/inviwoqtutils.h
    include/modules/qtwidgets/keyboardutils.h
    include/modules/qtwidgets/labelgraphicsitem.h
    include/modules/qtwidgets/mousecursorutils.h
    include/modules/qtwidgets/numberlineedit.h
    include/modules/qtwidgets/ordinalbasewidget.h
    include/modules/qtwidgets/processors/processorwidgetqt.h
    include/modules/qtwidgets/properties/anglepropertywidgetqt.h
    include/modules/qtwidgets/properties/boolcompositepropertywidgetqt.h
    include/modules/qtwidgets/properties/boolpropertywidgetqt.h
    include/modules/qtwidgets/properties/buttongrouppropertywidgetqt.h
    include/modules/qtwidgets/properties/buttonpropertywidgetqt.h
    include/modules/qtwidgets/properties/collapsiblegroupboxwidgetqt.h
    include/modules/qtwidgets/properties/colorpropertywidgetqt.h
    include/modules/qtwidgets/properties/compositepropertywidgetqt.h
    include/modules/qtwidgets/properties/eventpropertywidgetqt.h
    include/modules/qtwidgets/properties/filepropertywidgetqt.h
    include/modules/qtwidgets/properties/fontsizepropertywidgetqt.h
    include/modules/qtwidgets/properties/indicatorwidget.h
    include/modules/qtwidgets/properties/isotfpropertywidgetqt.h
    include/modules/qtwidgets/properties/isovaluepropertywidgetqt.h
    include/modules/qtwidgets/properties/lightpropertywidgetqt.h
    include/modules/qtwidgets/properties/listpropertywidgetqt.h
    include/modules/qtwidgets/properties/multifilepropertywidgetqt.h
    include/modules/qtwidgets/properties/optionpropertywidgetqt.h
    include/modules/qtwidgets/properties/ordinalminmaxpropertywidgetqt.h
    include/modules/qtwidgets/properties/ordinalminmaxtextpropertywidgetqt.h
    include/modules/qtwidgets/properties/ordinalpropertywidgetqt.h
    include/modules/qtwidgets/properties/propertyeditorwidgetqt.h
    include/modules/qtwidgets/properties/propertysettingswidgetqt.h
    include/modules/qtwidgets/properties/propertywidgetqt.h
    include/modules/qtwidgets/properties/stringpropertywidgetqt.h
    include/modules/qtwidgets/properties/stringspropertywidgetqt.h
    include/modules/qtwidgets/properties/texteditorwidgetqt.h
    include/modules/qtwidgets/properties/tfprimitivesetwidgetqt.h
    include/modules/qtwidgets/properties/tfpropertywidgetqt.h
    include/modules/qtwidgets/properties/valuedragger.h
    include/modules/qtwidgets/qstringhelper.h
    include/modules/qtwidgets/qtwidgetsmodule.h
    include/modules/qtwidgets/qtwidgetsmoduledefine.h
    include/modules/qtwidgets/rawdatareaderdialogqt.h
    include/modules/qtwidgets/textlabeloverlay.h
    include/modules/qtwidgets/tf/tfcontrolpointconnection.h
    include/modules/qtwidgets/tf/tfeditorcontrolpoint.h
    include/modules/qtwidgets/tf/tfeditorisovalue.h
    include/modules/qtwidgets/tf/tfeditorprimitive.h
    include/modules/qtwidgets/tf/tfeditorview.h
    include/modules/qtwidgets/tf/tfpropertyconcept.h
    include/modules/qtwidgets/tf/tfpropertydialog.h
    include/modules/qtwidgets/tf/tfutils.h
)
ivw_group("Header Files" ${HEADER_FILES})

# Add source files
set(SOURCE_FILES
    src/angleradiuswidget.cpp
    src/codeedit.cpp
    src/colorwheel.cpp
    src/customdoublespinboxqt.cpp
    src/editablelabelqt.cpp
    src/editorfileobserver.cpp
    src/editorsettings.cpp
    src/eventconverterqt.cpp
    src/filepathlineeditqt.cpp
    src/inviwodockwidget.cpp
    src/inviwodockwidgettitlebar.cpp
    src/inviwofiledialog.cpp
    src/inviwoqtutils.cpp
    src/inviwowidgetsqt.cpp
    src/keyboardutils.cpp
    src/labelgraphicsitem.cpp
    src/lightpositionwidgetqt.cpp
    src/lineeditqt.cpp
    src/mousecursorutils.cpp
    src/numberlineedit.cpp
    src/ordinalbasewidget.cpp
    src/ordinaleditorwidget.cpp
    src/processors/processordockwidgetqt.cpp
    src/processors/processorwidgetqt.cpp
    src/properties/anglepropertywidgetqt.cpp
    src/properties/boolcompositepropertywidgetqt.cpp
    src/properties/boolpropertywidgetqt.cpp
    src/properties/buttongrouppropertywidgetqt.cpp
    src/properties/buttonpropertywidgetqt.cpp
    src/properties/collapsiblegroupboxwidgetqt.cpp
    src/properties/colorlineedit.cpp
    src/properties/colorpropertywidgetqt.cpp
    src/properties/compositepropertywidgetqt.cpp
    src/properties/doublevaluedragspinbox.cpp
    src/properties/eventpropertywidgetqt.cpp
    src/properties/filepropertywidgetqt.cpp
    src/properties/fontsizepropertywidgetqt.cpp
    src/properties/indicatorwidget.cpp
    src/properties/isotfpropertywidgetqt.cpp
    src/properties/isovaluepropertywidgetqt.cpp
    src/properties/lightpropertywidgetqt.cpp
    src/properties/listpropertywidgetqt.cpp
    src/properties/multifilepropertywidgetqt.cpp
    src/properties/optionpropertywidgetqt.cpp
    src/properties/ordinalminmaxpropertywidgetqt.cpp
    src/properties/ordinalminmaxtextpropertywidgetqt.cpp
    src/properties/ordinalpropertywidgetqt.cpp
    src/properties/ordinalspinboxwidget.cpp
    src/properties/propertyeditorwidgetqt.cpp
    src/properties/propertysettingswidgetqt.cpp
    src/properties/propertywidgetqt.cpp
    src/properties/stringmultilinepropertywidgetqt.cpp
    src/properties/stringpropertywidgetqt.cpp
    src/properties/stringspropertywidgetqt.cpp
    src/properties/texteditorwidgetqt.cpp
    src/properties/tfprimitivesetwidgetqt.cpp
    src/properties/tfpropertywidgetqt.cpp
    src/properties/valuedragger.cpp
    src/properties/valuedragspinbox.cpp
    src/propertylistwidget.cpp
    src/qtwidgetsmodule.cpp
    src/rangesliderqt.cpp
    src/rawdatareaderdialogqt.cpp
    src/sliderwidgetqt.cpp
    src/syntaxhighlighter.cpp
    src/textlabeloverlay.cpp
    src/tf/tfcoloredit.cpp
    src/tf/tfcontrolpointconnection.cpp
    src/tf/tfeditor.cpp
    src/tf/tfeditorcontrolpoint.cpp
    src/tf/tfeditorisovalue.cpp
    src/tf/tfeditorprimitive.cpp
    src/tf/tfeditorview.cpp
    src/tf/tflineedit.cpp
    src/tf/tfpropertyconcept.cpp
    src/tf/tfpropertydialog.cpp
    src/tf/tfselectionwatcher.cpp
    src/tf/tfutils.cpp
    src/tfhelpwindow.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Add moc files
qt_wrap_cpp(MOCED_FILES ${MOC_FILES})
source_group("MOC Files" FILES ${MOCED_FILES})

# Add resource file
qt_add_resources(QRC_FILE "${IVW_RESOURCES_DIR}/inviwo.qrc")
source_group("Resource Files" FILES ${IVW_RESOURCES_DIR}/inviwo.qrc ${QRC_FILE})

# Create module
ivw_create_module(NO_PCH QT ${SOURCE_FILES} ${MOCED_FILES} ${QRC_FILE} ${HEADER_FILES}
    $<$<BOOL:${WIN32}>:${IVW_ROOT_DIR}/tools/natvis/qt5.natvis>
)
target_link_libraries(inviwo-module-qtwidgets PUBLIC
    inviwo::qtapplicationbase
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
)

# Qt headers used in PCH file as identified by CompileScore extension in Visual Studio.
# These will be combined with the PCH headers of inviwo-core
set(PCH_HEADERS_QT
    <QWidget>
    <QString>
)
ivw_compile_optimize_on_target(inviwo-module-qtwidgets HEADERS ${PCH_HEADERS_QT})

ivw_deploy_qt(inviwo-module-qtwidgets)

ivw_register_license_file(NAME "Qt" VERSION ${QtWidgets_VERSION} MODULE QtWidgets TYPE "LGPLv3"
    URL https://www.qt.io/
    FILES ${IVW_EXTENSIONS_DIR}/qt/LICENSE.txt
)
