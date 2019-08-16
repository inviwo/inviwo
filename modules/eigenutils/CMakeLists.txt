#--------------------------------------------------------------------
# Inviwo EigenUtils Module
ivw_module(EigenUtils)

#--------------------------------------------------------------------
# Add header files
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

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/eigenutilsmodule.cpp
    src/processors/eigenmatrixtoimage.cpp
    src/processors/eigenmix.cpp
    src/processors/eigennormalize.cpp
    src/processors/testmatrix.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Create module
ivw_create_module(NO_PCH ${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

set_target_properties(inviwo-module-eigenutils PROPERTIES COTIRE_PREFIX_HEADER_PUBLIC_IGNORE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/ext/Eigen")
ivw_compile_optimize_on_target(inviwo-module-eigenutils)

target_include_directories(inviwo-module-eigenutils PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ext/eigen>
)

ivw_register_license_file(NAME Eigen VERSION 3.3.90 MODULE EigenUtils
    URL http://eigen.tuxfamily.org/
    TYPE "Mozilla Public License Version 2.0"
    FILES 
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/eigen/COPYING.README
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/eigen/COPYING.MPL2
)
ivw_make_package(InviwoEigenUtilsModule inviwo-module-eigenutils)

#  HACK: have the files showing in the IDE, under the name 'eigen'
file(GLOB_RECURSE EIGEN_SOURCES "ext/eigen/Eigen/*")
ivw_group("Header Files" BASE "${CMAKE_CURRENT_SOURCE_DIR}/ext/eigen/Eigen" ${EIGEN_SOURCES})
add_custom_target(eigen_dummy SOURCES ${EIGEN_SOURCES})

# Creates VS folder structure
ivw_folder(eigen_dummy ext)
