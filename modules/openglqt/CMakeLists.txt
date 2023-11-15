# Inviwo OpenGLQt Module
ivw_module(OpenGLQt)

find_package(Qt6 REQUIRED COMPONENTS Svg OpenGL OpenGLWidgets)

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

qt_wrap_cpp(MOCED_FILES ${MOC_FILES})
source_group("MOC Files" FILES ${MOCED_FILES})

# Create module
ivw_create_module(NO_PCH ${SOURCE_FILES} ${MOCED_FILES} ${HEADER_FILES})
target_link_libraries(inviwo-module-openglqt PUBLIC 
    Qt6::OpenGL
    Qt6::Svg
    Qt6::OpenGLWidgets
)

ivw_deploy_qt(inviwo-module-openglqt)
