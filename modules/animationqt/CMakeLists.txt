# Inviwo AnimationQt Module
ivw_module(AnimationQt)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Add header files
set(HEADER_FILES
    include/modules/animationqt/animationeditordockwidgetqt.h
    include/modules/animationqt/animationeditorqt.h
    include/modules/animationqt/animationlabelviewqt.h
    include/modules/animationqt/animationqtmodule.h
    include/modules/animationqt/animationqtmoduledefine.h
    include/modules/animationqt/animationqtsupplier.h
    include/modules/animationqt/animationviewqt.h
    include/modules/animationqt/demo/demonavigatordockwidgetqt.h
    include/modules/animationqt/factories/sequenceeditorfactory.h
    include/modules/animationqt/factories/sequenceeditorfactoryobject.h
    include/modules/animationqt/factories/trackwidgetqtfactory.h
    include/modules/animationqt/factories/trackwidgetqtfactoryobject.h
    include/modules/animationqt/sequenceeditor/controlsequenceeditor.h
    include/modules/animationqt/sequenceeditor/invalidationsequenceeditor.h
    include/modules/animationqt/sequenceeditor/keyframeeditorwidget.h
    include/modules/animationqt/sequenceeditor/propertysequenceeditor.h
    include/modules/animationqt/sequenceeditor/sequenceeditorpanel.h
    include/modules/animationqt/sequenceeditor/sequenceeditorwidget.h
    include/modules/animationqt/trackcontrolswidgetqt.h
    include/modules/animationqt/widgets/controltrackwidgetqt.h
    include/modules/animationqt/widgets/editorconstants.h
    include/modules/animationqt/widgets/keyframesequencewidgetqt.h
    include/modules/animationqt/widgets/keyframewidgetqt.h
    include/modules/animationqt/widgets/propertytrackwidgetqt.h
    include/modules/animationqt/widgets/trackwidgetqt.h
    include/modules/animationqt/workspaceanimationsmodel.h
)
ivw_group("Header Files" ${HEADER_FILES})

# Add source files
set(SOURCE_FILES
    src/animationeditordockwidgetqt.cpp
    src/animationeditorqt.cpp
    src/animationlabelviewqt.cpp
    src/animationqtmodule.cpp
    src/animationqtsupplier.cpp
    src/animationviewqt.cpp
    src/demo/demonavigatordockwidgetqt.cpp
    src/factories/sequenceeditorfactory.cpp
    src/factories/sequenceeditorfactoryobject.cpp
    src/factories/trackwidgetqtfactory.cpp
    src/factories/trackwidgetqtfactoryobject.cpp
    src/sequenceeditor/controlsequenceeditor.cpp
    src/sequenceeditor/invalidationsequenceeditor.cpp
    src/sequenceeditor/keyframeeditorwidget.cpp
    src/sequenceeditor/propertysequenceeditor.cpp
    src/sequenceeditor/sequenceeditorpanel.cpp
    src/sequenceeditor/sequenceeditorwidget.cpp
    src/trackcontrolswidgetqt.cpp
    src/widgets/controltrackwidgetqt.cpp
    src/widgets/editorconstants.cpp
    src/widgets/keyframesequencewidgetqt.cpp
    src/widgets/keyframewidgetqt.cpp
    src/widgets/propertytrackwidgetqt.cpp
    src/widgets/trackwidgetqt.cpp
    src/workspaceanimationsmodel.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Add some qt specific things
qt_add_resources(QRC_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources/animation.qrc")
source_group("Resource Files" FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/animation.qrc ${QRC_FILE})

# Create module
ivw_create_module(QT ${SOURCE_FILES} ${HEADER_FILES} ${QRC_FILE})
target_link_libraries(inviwo-module-animationqt PUBLIC 
    Qt6::Core 
    Qt6::Gui
    Qt6::Widgets
)


