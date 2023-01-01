ivw_module(MeshAndVolume)

set(HEADER_FILES
    include/inviwo/meshandvolume/meshandvolumemodule.h
    include/inviwo/meshandvolume/meshandvolumemoduledefine.h
    include/inviwo/meshandvolume/processors/mvrenderer.h
    include/inviwo/meshandvolume/processors/volumerasterizer.h
    include/inviwo/meshandvolume/rendering/myfragmentlistrenderer.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/meshandvolumemodule.cpp
    src/processors/mvrenderer.cpp
    src/processors/volumerasterizer.cpp
    src/rendering/myfragmentlistrenderer.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

set(SHADER_FILES
	# Add shaders
    glsl/myvolumerasterizer.vert
    glsl/myvolumerasterizer.frag
    glsl/oit/myabufferlinkedlist.glsl
    glsl/oit/myclear.frag
    glsl/oit/mycommons.glsl
    glsl/oit/mydisplay.frag
    glsl/oit/simplequad.vert
    glsl/oit/mysort.glsl
)
ivw_group("Shader Files" ${SHADER_FILES})

set(TEST_FILES
    tests/unittests/meshandvolume-unittest-main.cpp
)
ivw_add_unittest(${TEST_FILES})

ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

# Add shader directory to install package
ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)
