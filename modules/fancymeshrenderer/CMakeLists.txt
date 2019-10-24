#--------------------------------------------------------------------
# Inviwo fancymeshrenderer Module
ivw_module(FancyMeshRenderer)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    #fancymeshrendererprocessor.h
    include/modules/fancymeshrenderer/HalfEdges.h
    include/modules/fancymeshrenderer/calcnormals.h
    include/modules/fancymeshrenderer/processors/calcnormalsprocessor.h
    include/modules/fancymeshrenderer/processors/FragmentListRenderer.h
    include/modules/fancymeshrenderer/processors/fancymeshrenderer.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    #fancymeshrendererprocessor.cpp
    src/HalfEdges.cpp
    src/calcnormals.cpp
    src/processors/calcnormalsprocessor.cpp
    src/processors/FragmentListRenderer.cpp
    src/processors/fancymeshrenderer.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
    #glsl/fancymeshrendererprocessor.frag
    glsl/fancymeshrenderer.frag
    glsl/fancymeshrenderer.geom
    glsl/fancymeshrenderer.vert
    glsl/ABufferCommons.hglsl
    glsl/ABufferLinkedList.hglsl
    glsl/ABufferSort.hglsl
    glsl/clearABufferLinkedList.frag
    glsl/dispABufferLinkedList.frag
    glsl/simpleQuad.vert
    glsl/depthOnly.frag
    glsl/IllustrationBuffer.hglsl
    glsl/SortAndFillIllustrationBuffer.frag
    glsl/DisplayIllustrationBuffer.frag
    glsl/ResolveNeighborsIllustrationBuffer.frag
    glsl/SmoothIllustrationBuffer.frag
)
ivw_group("Shader Files" ${SHADER_FILES})


#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
	tests/unittests/fancymeshrenderer-unittest-main.cpp
    tests/unittests/halfedges-test.cpp
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

#--------------------------------------------------------------------
# Add shader directory to pack
ivw_add_to_module_pack(glsl)