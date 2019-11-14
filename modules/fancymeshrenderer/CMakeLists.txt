#--------------------------------------------------------------------
# Inviwo fancymeshrenderer Module
ivw_module(FancyMeshRenderer)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/modules/fancymeshrenderer/halfedges.h
    include/modules/fancymeshrenderer/calcnormals.h
    include/modules/fancymeshrenderer/processors/calcnormalsprocessor.h
    include/modules/fancymeshrenderer/processors/fragmentlistrenderer.h
    include/modules/fancymeshrenderer/processors/fancymeshrenderer.h
    include/modules/fancymeshrenderer/fancymeshrenderermodule.h
    include/modules/fancymeshrenderer/fancymeshrenderermoduledefine.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/halfedges.cpp
    src/calcnormals.cpp
    src/processors/calcnormalsprocessor.cpp
    src/processors/fragmentlistrenderer.cpp
    src/processors/fancymeshrenderer.cpp
    src/fancymeshrenderermodule.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
    glsl/fancymeshrenderer.frag
    glsl/fancymeshrenderer.geom
    glsl/fancymeshrenderer.vert
    glsl/abuffercommons.hglsl
    glsl/abufferlinkedList.hglsl
    glsl/abuffersort.hglsl
    glsl/clearabufferlinkedlist.frag
    glsl/dispabufferlinkedlist.frag
    glsl/simplequad.vert
    glsl/depthonly.frag
    glsl/illustrationbuffer.hglsl
    glsl/sortandfillillustrationbuffer.frag
    glsl/displayIllustrationbuffer.frag
    glsl/resolveneighborsillustrationbuffer.frag
    glsl/smoothillustrationbuffer.frag
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
