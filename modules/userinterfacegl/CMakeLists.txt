#--------------------------------------------------------------------
# Inviwo UserInterfaceGL Module
ivw_module(UserInterfaceGL)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/modules/userinterfacegl/glui/element.h
    include/modules/userinterfacegl/glui/layout/boxlayout.h
    include/modules/userinterfacegl/glui/layout/hboxlayout.h
    include/modules/userinterfacegl/glui/layout/layout.h
    include/modules/userinterfacegl/glui/layout/vboxlayout.h
    include/modules/userinterfacegl/glui/renderer.h
    include/modules/userinterfacegl/glui/widgetfactory.h
    include/modules/userinterfacegl/glui/widgetfactoryobject.h
    include/modules/userinterfacegl/glui/widgets/abstractbutton.h
    include/modules/userinterfacegl/glui/widgets/boolpropertywidget.h
    include/modules/userinterfacegl/glui/widgets/button.h
    include/modules/userinterfacegl/glui/widgets/buttonpropertywidget.h
    include/modules/userinterfacegl/glui/widgets/checkbox.h
    include/modules/userinterfacegl/glui/widgets/doubleminmaxpropertywidget.h
    include/modules/userinterfacegl/glui/widgets/floatminmaxpropertywidget.h
    include/modules/userinterfacegl/glui/widgets/floatpropertywidget.h
    include/modules/userinterfacegl/glui/widgets/intminmaxpropertywidget.h
    include/modules/userinterfacegl/glui/widgets/intpropertywidget.h
    include/modules/userinterfacegl/glui/widgets/rangeslider.h
    include/modules/userinterfacegl/glui/widgets/slider.h
    include/modules/userinterfacegl/glui/widgets/toolbutton.h
    include/modules/userinterfacegl/glui/widgetsupplier.h
    include/modules/userinterfacegl/processors/camerawidget.h
    include/modules/userinterfacegl/processors/cropwidget.h
    include/modules/userinterfacegl/processors/gluiprocessor.h
    include/modules/userinterfacegl/processors/gluitestprocessor.h
    include/modules/userinterfacegl/processors/presentationprocessor.h
    include/modules/userinterfacegl/userinterfaceglmodule.h
    include/modules/userinterfacegl/userinterfaceglmoduledefine.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/glui/element.cpp
    src/glui/layout/boxlayout.cpp
    src/glui/layout/hboxlayout.cpp
    src/glui/layout/layout.cpp
    src/glui/layout/vboxlayout.cpp
    src/glui/renderer.cpp
    src/glui/widgetfactory.cpp
    src/glui/widgetfactoryobject.cpp
    src/glui/widgets/abstractbutton.cpp
    src/glui/widgets/boolpropertywidget.cpp
    src/glui/widgets/button.cpp
    src/glui/widgets/buttonpropertywidget.cpp
    src/glui/widgets/checkbox.cpp
    src/glui/widgets/doubleminmaxpropertywidget.cpp
    src/glui/widgets/floatminmaxpropertywidget.cpp
    src/glui/widgets/floatpropertywidget.cpp
    src/glui/widgets/intminmaxpropertywidget.cpp
    src/glui/widgets/intpropertywidget.cpp
    src/glui/widgets/rangeslider.cpp
    src/glui/widgets/slider.cpp
    src/glui/widgets/toolbutton.cpp
    src/glui/widgetsupplier.cpp
    src/processors/camerawidget.cpp
    src/processors/cropwidget.cpp
    src/processors/gluiprocessor.cpp
    src/processors/gluitestprocessor.cpp
    src/processors/presentationprocessor.cpp
    src/userinterfaceglmodule.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/geometrycustompicking.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/labelui.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/renderui.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/renderui.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/widgetrenderer.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/widgettexture.frag
)
ivw_group("Shader Files" ${SHADER_FILES})


#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
    #${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/userinterfacegl-test.cpp
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

#--------------------------------------------------------------------
# Add shader and data directories to pack
ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)
ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/data)
