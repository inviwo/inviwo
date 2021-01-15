#--------------------------------------------------------------------
# Inviwo fancymeshrenderer Module
ivw_module(MeshRenderingGL)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/modules/meshrenderinggl/datastructures/halfedges.h
    include/modules/meshrenderinggl/datastructures/rasterization.h
    include/modules/meshrenderinggl/datastructures/transformedrasterization.h
    include/modules/meshrenderinggl/algorithm/calcnormals.h
    include/modules/meshrenderinggl/ports/rasterizationport.h
    include/modules/meshrenderinggl/processors/calcnormalsprocessor.h
    include/modules/meshrenderinggl/processors/linerasterizer.h
    include/modules/meshrenderinggl/processors/meshrasterizer.h
    include/modules/meshrenderinggl/processors/rasterizationrenderer.h
    include/modules/meshrenderinggl/processors/transformrasterization.h
    include/modules/meshrenderinggl/rendering/fragmentlistrenderer.h
    include/modules/meshrenderinggl/meshrenderingglmodule.h
    include/modules/meshrenderinggl/meshrenderingglmoduledefine.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/datastructures/halfedges.cpp
    src/datastructures/rasterization.cpp
    src/datastructures/transformedrasterization.cpp
    src/algorithm/calcnormals.cpp
    src/processors/calcnormalsprocessor.cpp
    src/ports/rasterizationport.cpp
    src/processors/linerasterizer.cpp
    src/processors/meshrasterizer.cpp
    src/processors/rasterizationrenderer.cpp
    src/processors/transformrasterization.cpp
    src/rendering/fragmentlistrenderer.cpp
    src/meshrenderingglmodule.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
    glsl/fancymeshrenderer.frag
    glsl/fancymeshrenderer.geom
    glsl/fancymeshrenderer.vert
    glsl/illustration/display.frag
    glsl/illustration/illustrationbuffer.glsl
    glsl/illustration/neighbors.frag
    glsl/illustration/smooth.frag
    glsl/illustration/sortandfill.frag
    glsl/oit/abufferlinkedlist.glsl
    glsl/oit/clear.frag
    glsl/oit/commons.glsl
    glsl/oit/display.frag
    glsl/oit/simplequad.vert
    glsl/oit/sort.glsl
    glsl/oit-linerenderer.frag
)
ivw_group("Shader Files" ${SHADER_FILES})


#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
    tests/unittests/compresscolor-test.cpp
    tests/unittests/halfedges-test.cpp
    tests/unittests/meshrenderinggl-unittest-main.cpp
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

#--------------------------------------------------------------------
# Add shader directory to pack
ivw_add_to_module_pack(glsl)

