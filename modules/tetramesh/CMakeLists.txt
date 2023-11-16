ivw_module(TetraMesh)

set(HEADER_FILES
    include/inviwo/tetramesh/datastructures/tetramesh.h
    include/inviwo/tetramesh/datastructures/tetrameshbuffers.h
    include/inviwo/tetramesh/datastructures/volumetetramesh.h
    include/inviwo/tetramesh/ports/tetrameshport.h
    include/inviwo/tetramesh/processors/tetrameshboundaryextractor.h
    include/inviwo/tetramesh/processors/tetrameshboundingbox.h
    include/inviwo/tetramesh/processors/tetrameshvolumeraycaster.h
    include/inviwo/tetramesh/processors/transformtetramesh.h
    include/inviwo/tetramesh/processors/volumetotetramesh.h
    include/inviwo/tetramesh/tetrameshmodule.h
    include/inviwo/tetramesh/tetrameshmoduledefine.h
    include/inviwo/tetramesh/util/tetrameshutils.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/datastructures/tetramesh.cpp
    src/datastructures/tetrameshbuffers.cpp
    src/datastructures/volumetetramesh.cpp
    src/ports/tetrameshport.cpp
    src/processors/tetrameshboundaryextractor.cpp
    src/processors/tetrameshboundingbox.cpp
    src/processors/tetrameshvolumeraycaster.cpp
    src/processors/transformtetramesh.cpp
    src/processors/volumetotetramesh.cpp
    src/tetrameshmodule.cpp
    src/util/tetrameshutils.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

set(SHADER_FILES
    glsl/tetramesh_traversal.vert
    glsl/tetramesh_traversal.frag
)
ivw_group("Shader Files" ${SHADER_FILES})

ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)
