#--------------------------------------------------------------------
# Inviwo AnimationQt Module
ivw_module(AnimationQt)

find_package(Qt5Core QUIET REQUIRED)
find_package(Qt5Gui QUIET REQUIRED)
find_package(Qt5Widgets QUIET REQUIRED)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/animationeditordockwidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/animationeditorqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/animationlabelviewqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/animationqtsupplier.h
    ${CMAKE_CURRENT_SOURCE_DIR}/animationviewqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/demo/demonavigatordockwidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/factories/sequenceeditorfactory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/factories/sequenceeditorfactoryobject.h
    ${CMAKE_CURRENT_SOURCE_DIR}/factories/trackwidgetqtfactory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/factories/trackwidgetqtfactoryobject.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sequenceeditor/controlsequenceeditor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sequenceeditor/propertysequenceeditor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sequenceeditor/sequenceeditorpanel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sequenceeditor/sequenceeditorwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/trackcontrolswidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/controltrackwidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/editorconstants.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/keyframesequencewidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/keyframewidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/propertytrackwidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/trackwidgetqt.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/animationeditordockwidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/animationeditorqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/animationlabelviewqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/animationqtsupplier.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/animationviewqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/demo/demonavigatordockwidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/factories/sequenceeditorfactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/factories/sequenceeditorfactoryobject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/factories/trackwidgetqtfactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/factories/trackwidgetqtfactoryobject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sequenceeditor/controlsequenceeditor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sequenceeditor/propertysequenceeditor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sequenceeditor/sequenceeditorpanel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sequenceeditor/sequenceeditorwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/trackcontrolswidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/controltrackwidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/editorconstants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/keyframesequencewidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/keyframewidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/propertytrackwidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/trackwidgetqt.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Add some qt specific things
qt5_add_resources(QRC_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources/animation.qrc")
source_group("Resource Files" FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/animation.qrc 
    ${QRC_FILE}
)

#--------------------------------------------------------------------
# Create module
ivw_create_module(NO_PCH ${SOURCE_FILES} ${HEADER_FILES} ${QRC_FILE})
target_link_libraries(inviwo-module-animationqt PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets)

#ivw_compile_optimize_on_target(inviwo-module-animationqt)

ivw_make_package(InviwoAnimationQtModule inviwo-module-animationqt)

