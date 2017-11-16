#--------------------------------------------------------------------
# Inviwo fancymeshrenderer Module
ivw_module(FancyMeshRenderer)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    #${CMAKE_CURRENT_SOURCE_DIR}/fancymeshrendererprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/fancymeshrenderer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/FragmentListRenderer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/HalfEdges.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    #${CMAKE_CURRENT_SOURCE_DIR}/fancymeshrendererprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/fancymeshrenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/FragmentListRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/HalfEdges.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
    #${CMAKE_CURRENT_SOURCE_DIR}/glsl/fancymeshrendererprocessor.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/fancymeshrenderer.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/fancymeshrenderer.geom
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/fancymeshrenderer.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/ABufferLinkedList.hglsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/ABufferSort.hglsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/clearABufferLinkedList.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/dispABufferLinkedList.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/simpleQuad.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/depthOnly.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/IllustrationBuffer.hglsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/SortAndFillIllustrationBuffer.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/DisplayIllustrationBuffer.frag
)
ivw_group("Shader Files" ${SHADER_FILES})


#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/fancymeshrenderer-unittest-main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/halfedges-test.cpp
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

#--------------------------------------------------------------------
# Add shader directory to pack
ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)