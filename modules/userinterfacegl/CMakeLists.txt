#--------------------------------------------------------------------
# Inviwo UserInterfaceGL Module
ivw_module(UserInterfaceGL)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/element.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/layout/boxlayout.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/layout/hboxlayout.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/layout/layout.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/layout/vboxlayout.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/renderer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/widgets/abstractbutton.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/widgets/boolpropertywidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/widgets/button.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/widgets/buttonpropertywidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/widgets/checkbox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/widgets/floatminmaxpropertywidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/widgets/floatpropertywidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/widgets/intminmaxpropertywidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/widgets/intpropertywidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/widgets/rangeslider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/widgets/slider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/widgets/toolbutton.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/camerawidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/cropwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/gluitestprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/presentationprocessor.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/element.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/layout/boxlayout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/layout/hboxlayout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/layout/layout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/layout/vboxlayout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/renderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/widgets/abstractbutton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/widgets/boolpropertywidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/widgets/button.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/widgets/buttonpropertywidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/widgets/checkbox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/widgets/floatminmaxpropertywidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/widgets/floatpropertywidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/widgets/intminmaxpropertywidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/widgets/intpropertywidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/widgets/rangeslider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/widgets/slider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glui/widgets/toolbutton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/camerawidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/cropwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/gluitestprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/presentationprocessor.cpp
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
