# Inviwo assimp Module
ivw_module(Assimp)

set(HEADER_FILES
    include/modules/assimp/assimpmodule.h
    include/modules/assimp/assimpmoduledefine.h
    include/modules/assimp/assimpreader.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/assimpmodule.cpp
    src/assimpreader.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})

find_package(assimp CONFIG REQUIRED)
ivw_vcpkg_install(assimp MODULE Assimp)

target_link_libraries(inviwo-module-assimp PUBLIC assimp::assimp)
