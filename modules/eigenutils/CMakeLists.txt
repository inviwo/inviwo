# Inviwo EigenUtils Module
ivw_module(EigenUtils)

set(HEADER_FILES
    include/modules/eigenutils/eigenports.h
    include/modules/eigenutils/eigenutils.h
    include/modules/eigenutils/eigenutilsmodule.h
    include/modules/eigenutils/eigenutilsmoduledefine.h
    include/modules/eigenutils/processors/eigenmatrixtoimage.h
    include/modules/eigenutils/processors/eigenmix.h
    include/modules/eigenutils/processors/eigennormalize.h
    include/modules/eigenutils/processors/testmatrix.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/eigenutilsmodule.cpp
    src/processors/eigenmatrixtoimage.cpp
    src/processors/eigenmix.cpp
    src/processors/eigennormalize.cpp
    src/processors/testmatrix.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

find_package(Eigen3 REQUIRED)
target_link_libraries(inviwo-module-eigenutils PUBLIC Eigen3::Eigen)
ivw_vcpkg_install(eigen3 MODULE EigenUtils)
