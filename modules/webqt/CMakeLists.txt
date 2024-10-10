ivw_module(WebQt)

set(HEADER_FILES
    include/inviwo/webqt/htmlsyntaxhighlight.h
    include/inviwo/webqt/javascriptsyntaxhighlight.h
    include/inviwo/webqt/properties/htmlpropertywidgetqt.h
    include/inviwo/webqt/properties/javascriptpropertywidgetqt.h
    include/inviwo/webqt/webqtmodule.h
    include/inviwo/webqt/webqtmoduledefine.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/htmlsyntaxhighlight.cpp
    src/javascriptsyntaxhighlight.cpp
    src/properties/htmlpropertywidgetqt.cpp
    src/properties/javascriptpropertywidgetqt.cpp
    src/webqtmodule.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

set(SHADER_FILES
	# Add shaders
)
ivw_group("Shader Files" ${SHADER_FILES})

set(TEST_FILES
    tests/unittests/webqt-unittest-main.cpp
)
ivw_add_unittest(${TEST_FILES})

ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

# Add shader directory to install package
#ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)
