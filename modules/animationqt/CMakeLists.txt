#--------------------------------------------------------------------
# Inviwo AnimationQt Module
ivw_module(AnimationQt)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    #${CMAKE_CURRENT_SOURCE_DIR}/animationqtprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/animationeditordockwidgetqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/animationeditorqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/animationviewqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/keyframeqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/keyframesequenceqt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/trackqt.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    #${CMAKE_CURRENT_SOURCE_DIR}/animationqtprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/animationeditordockwidgetqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/animationeditorqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/animationviewqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/keyframeqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/keyframesequenceqt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/trackqt.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
    #${CMAKE_CURRENT_SOURCE_DIR}/glsl/animationqtprocessor.frag
)
ivw_group("Shader Files" ${SHADER_FILES})


#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
	#${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/animationqt-test.cpp
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

#--------------------------------------------------------------------
# Add shader directory to pack
# ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)