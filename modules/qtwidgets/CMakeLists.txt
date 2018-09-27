#--------------------------------------------------------------------
# Inviwo Qt Widgets
ivw_module(QtWidgets)

find_package(Qt5Core QUIET REQUIRED)
find_package(Qt5Gui QUIET REQUIRED)
find_package(Qt5Widgets QUIET REQUIRED)

#-------------------------------------------------------------------- 
# Add MOC files
set(MOC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/angleradiuswidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/colorwheel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/customdoublespinboxqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/editablelabelqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/filepathlineeditqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/inviwodockwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/inviwodockwidgettitlebar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/inviwofiledialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/inviwowidgetsqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/lightpositionwidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/lineeditqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ordinaleditorwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/processordockwidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/processorwidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/colorlineedit.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/doublevaluedragspinbox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/ordinalspinboxwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/stringmultilinepropertywidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/valuedragspinbox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/propertylistwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/rangesliderqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sliderwidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tf/tfcoloredit.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tf/tfeditor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tf/tflineedit.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tf/tfselectionwatcher.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tfhelpwindow.h
)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/codeedit.h
    ${CMAKE_CURRENT_SOURCE_DIR}/eventconverterqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/inviwoqtutils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/keyboardutils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/labelgraphicsitem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ordinalbasewidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/anglepropertywidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/boolcompositepropertywidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/boolpropertywidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/buttonpropertywidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/collapsiblegroupboxwidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/colorpropertywidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/compositepropertywidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/eventpropertywidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/filepropertywidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/fontsizepropertywidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/indicatorwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/isotfpropertywidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/isovaluepropertywidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/lightpropertywidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/listpropertywidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/multifilepropertywidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/optionpropertywidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/ordinalminmaxpropertywidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/ordinalminmaxtextpropertywidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/ordinalpropertywidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/propertyeditorwidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/propertysettingswidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/propertywidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/stringpropertywidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/syntaxhighlighter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/texteditorwidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/tfprimitivesetwidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/tfpropertywidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/valuedragger.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qstringhelper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtwidgetsmodule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtwidgetsmoduledefine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtwidgetssettings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/rawdatareaderdialogqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tf/tfcontrolpointconnection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tf/tfeditorcontrolpoint.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tf/tfeditorisovalue.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tf/tfeditorprimitive.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tf/tfeditorview.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tf/tfpropertydialog.h
    ${MOC_FILES}
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/angleradiuswidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codeedit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/colorwheel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/customdoublespinboxqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/editablelabelqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/eventconverterqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filepathlineeditqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/inviwodockwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/inviwodockwidgettitlebar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/inviwofiledialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/inviwoqtutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/inviwowidgetsqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/keyboardutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/labelgraphicsitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lightpositionwidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lineeditqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ordinalbasewidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ordinaleditorwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/processordockwidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/processorwidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/anglepropertywidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/boolcompositepropertywidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/boolpropertywidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/buttonpropertywidgetqt.cpp  
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/collapsiblegroupboxwidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/colorlineedit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/colorpropertywidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/compositepropertywidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/doublevaluedragspinbox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/eventpropertywidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/filepropertywidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/fontsizepropertywidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/glslsyntaxhighlighter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/indicatorwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/isotfpropertywidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/isovaluepropertywidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/lightpropertywidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/listpropertywidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/multifilepropertywidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/optionpropertywidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/ordinalminmaxpropertywidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/ordinalminmaxtextpropertywidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/ordinalpropertywidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/ordinalspinboxwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/propertyeditorwidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/propertysettingswidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/propertywidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/pythonsyntaxhighlighter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/stringmultilinepropertywidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/stringpropertywidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/syntaxhighlighter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/texteditorwidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/tfprimitivesetwidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/tfpropertywidgetqt.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/valuedragger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/valuedragspinbox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/propertylistwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtwidgetsmodule.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtwidgetssettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rangesliderqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rawdatareaderdialogqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sliderwidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tf/tfcoloredit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tf/tfcontrolpointconnection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tf/tfeditor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tf/tfeditorcontrolpoint.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tf/tfeditorisovalue.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tf/tfeditorprimitive.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tf/tfeditorview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tf/tflineedit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tf/tfpropertydialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tf/tfselectionwatcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tfhelpwindow.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Add moc files
qt5_wrap_cpp(MOCED_FILES ${MOC_FILES})
source_group("MOC Files" FILES ${MOCED_FILES})

# Add resource file
qt5_add_resources(QRC_FILE "${IVW_RESOURCES_DIR}/inviwo.qrc")
source_group("Resource Files" FILES 
    ${IVW_RESOURCES_DIR}/inviwo.qrc
    ${QRC_FILE}	
)

#--------------------------------------------------------------------
# Create module
ivw_create_module(NO_PCH ${SOURCE_FILES} ${MOCED_FILES} ${QRC_FILE} ${HEADER_FILES})
target_link_libraries(inviwo-module-qtwidgets PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets)

ivw_compile_optimize_on_target(inviwo-module-qtwidgets)

ivw_make_package(InviwoQtWidgetsModule inviwo-module-qtwidgets)
ivw_qt_add_to_install(qt_widgets Qt5Core Qt5Gui Qt5Widgets)

ivw_register_license_file(NAME "Qt" VERSION ${Qt5Widgets_VERSION} MODULE QtWidgets TYPE "LGPLv3"
    URL https://www.qt.io/
    FILES ${IVW_EXTENSIONS_DIR}/qt/LICENSE.txt
)

