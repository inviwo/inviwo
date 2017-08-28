#--------------------------------------------------------------------
# Inviwo UserInterfaceGL Module
ivw_module(UserInterfaceGL)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/camerawidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/cropwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/presentationprocessor.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/camerawidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/cropwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/presentationprocessor.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/geometrycustompicking.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/widgetrenderer.vert
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
