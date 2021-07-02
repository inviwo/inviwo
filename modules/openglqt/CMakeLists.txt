# Inviwo OpenGLQt Module
ivw_module(OpenGLQt)

# https://doc-snapshots.qt.io/qt6-dev/cmake-qt5-and-qt6-compatibility.html#supporting-older-qt-5-versions
find_package(QT NAMES Qt6 Qt5 COMPONENTS Svg OpenGL REQUIRED)
if(QT_VERSION_MAJOR EQUAL 6)
    find_package(Qt6 COMPONENTS Svg OpenGL OpenGLWidgets REQUIRED)
else()
    find_package(Qt5 COMPONENTS Svg OpenGL REQUIRED)
endif()
# Add MOC files
set(MOC_FILES
    include/modules/openglqt/openglqtmenu.h
)

# Add header files
set(HEADER_FILES
    ${MOC_FILES}
    include/modules/openglqt/canvasqopenglwidget.h
    include/modules/openglqt/glslsyntaxhighlight.h
    include/modules/openglqt/hiddencanvasqt.h
    include/modules/openglqt/interactioneventmapperqt.h
    include/modules/openglqt/openglqtcapabilities.h
    include/modules/openglqt/openglqtmodule.h
    include/modules/openglqt/openglqtmoduledefine.h
    include/modules/openglqt/processors/canvaswithpropertiesprocessorwidgetqt.h
    include/modules/openglqt/processors/canvaswithpropertiesprocessor.h
    include/modules/openglqt/processors/canvasprocessorwidgetqt.h
    include/modules/openglqt/properties/glsleditordockwidget.h
    include/modules/openglqt/properties/glslfilepropertywidgetqt.h
    include/modules/openglqt/properties/glslpropertywidgetqt.h
    include/modules/openglqt/shaderwidget.h
)
ivw_group("Header Files" ${HEADER_FILES})

# Add source files
set(SOURCE_FILES
    src/canvasqopenglwidget.cpp
    src/glslsyntaxhighlight.cpp
    src/hiddencanvasqt.cpp
    src/interactioneventmapperqt.cpp
    src/openglqtcapabilities.cpp
    src/openglqtmenu.cpp
    src/openglqtmodule.cpp
    src/processors/canvasprocessorwidgetqt.cpp
    src/processors/canvaswithpropertiesprocessor.cpp
    src/processors/canvaswithpropertiesprocessorwidgetqt.cpp
    src/properties/glsleditordockwidget.cpp
    src/properties/glslfilepropertywidgetqt.cpp
    src/properties/glslpropertywidgetqt.cpp
    src/shaderwidget.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_wrap_cpp(MOCED_FILES ${MOC_FILES})
else()
    qt5_wrap_cpp(MOCED_FILES ${MOC_FILES})
endif()
source_group("MOC Files" FILES ${MOCED_FILES})

# Create module
ivw_create_module(NO_PCH ${SOURCE_FILES} ${MOCED_FILES} ${HEADER_FILES})
target_link_libraries(inviwo-module-openglqt PRIVATE 
    Qt${QT_VERSION_MAJOR}::OpenGL
    Qt${QT_VERSION_MAJOR}::Svg
)
if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(inviwo-module-openglqt PRIVATE Qt6::OpenGLWidgets)
endif()

target_compile_definitions(inviwo-module-openglqt PUBLIC 
    "QT_DEPRECATED_WARNINGS_SINCE=QT_VERSION_CHECK(5, 15, 0)"
)

ivw_deploy_qt(inviwo-module-openglqt)

ivw_make_package(InviwoOpenGLQtModule inviwo-module-openglqt)
ivw_qt_add_to_install(qt_modules Qt${QT_VERSION_MAJOR}OpenGL)
