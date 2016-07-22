#--------------------------------------------------------------------
# Inviwo BrushingAndLinking Module
ivw_module(BrushingAndLinking)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    #${CMAKE_CURRENT_SOURCE_DIR}/brushingandlinkingprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/brushingandlinkingmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/indexlist.h
    ${CMAKE_CURRENT_SOURCE_DIR}/events/brushingandlinkingevent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/events/filteringevent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/events/selectionevent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ports/brushingandlinkingports.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/brushingandlinkingprocessor.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    #${CMAKE_CURRENT_SOURCE_DIR}/brushingandlinkingprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/brushingandlinkingmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/indexlist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/events/brushingandlinkingevent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/events/filteringevent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/events/selectionevent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ports/brushingandlinkingports.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/brushingandlinkingprocessor.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
    #${CMAKE_CURRENT_SOURCE_DIR}/glsl/brushingandlinkingprocessor.frag
)
ivw_group("Shader Files" ${SHADER_FILES})


#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
	#${CMAKE_CURRENT_SOURCE_DIR}/tests/brushingandlinking-test.cpp
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

#--------------------------------------------------------------------
# Add shader directory to pack
# ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)