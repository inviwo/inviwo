#--------------------------------------------------------------------
# Inviwo BrushingAndLinking Module
ivw_module(BrushingAndLinking)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    #${CMAKE_CURRENT_SOURCE_DIR}/brushingandlinkingprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/brushingandlinkingmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/indexlist.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/signedindexlist.h
    ${CMAKE_CURRENT_SOURCE_DIR}/events/brushingandlinkingevent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/events/signedbrushingandlinkingevent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/events/clusterselectionevent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/events/filteringevent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/events/selectionevent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/events/someotherselectionevent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ports/brushingandlinkingports.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/brushingandlinkingprocessor.h
    include/modules/brushingandlinking/brushingandlinkingmanager.h
    include/modules/brushingandlinking/brushingandlinkingmodule.h
    include/modules/brushingandlinking/brushingandlinkingmoduledefine.h
    include/modules/brushingandlinking/datastructures/indexlist.h
    include/modules/brushingandlinking/events/brushingandlinkingevent.h
    include/modules/brushingandlinking/events/filteringevent.h
    include/modules/brushingandlinking/events/selectionevent.h
    include/modules/brushingandlinking/ports/brushingandlinkingports.h
    include/modules/brushingandlinking/processors/brushingandlinkingprocessor.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    #${CMAKE_CURRENT_SOURCE_DIR}/brushingandlinkingprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/brushingandlinkingmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/indexlist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/signedindexlist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/events/brushingandlinkingevent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/events/signedbrushingandlinkingevent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/events/clusterselectionevent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/events/filteringevent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/events/selectionevent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/events/someotherselectionevent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ports/brushingandlinkingports.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/brushingandlinkingprocessor.cpp
    src/brushingandlinkingmanager.cpp
    src/brushingandlinkingmodule.cpp
    src/datastructures/indexlist.cpp
    src/events/brushingandlinkingevent.cpp
    src/events/filteringevent.cpp
    src/events/selectionevent.cpp
    src/ports/brushingandlinkingports.cpp
    src/processors/brushingandlinkingprocessor.cpp
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
