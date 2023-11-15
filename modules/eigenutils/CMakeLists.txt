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
ivw_create_module(NO_PCH ${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

set_target_properties(inviwo-module-eigenutils PROPERTIES COTIRE_PREFIX_HEADER_PUBLIC_IGNORE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/ext/Eigen")
ivw_compile_optimize_on_target(inviwo-module-eigenutils)

option(IVW_USE_EXTERNAL_EIGEN "Eigen is provided externaly" OFF)
if(NOT IVW_USE_EXTERNAL_EIGEN)
    target_include_directories(inviwo-module-eigenutils PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ext/eigengitlab>
    )
    #  HACK: have the files showing in the IDE, under the name 'eigen'
    file(GLOB_RECURSE EIGEN_SOURCES "ext/eigengitlab/Eigen/*")
    ivw_group("Header Files" BASE "${CMAKE_CURRENT_SOURCE_DIR}/ext/eigengitlab/Eigen" ${EIGEN_SOURCES})
    add_custom_target(eigen_dummy SOURCES ${EIGEN_SOURCES})

    # Creates VS folder structure
    ivw_folder(eigen_dummy ext)
    ivw_register_license_file(NAME Eigen VERSION 3.4.90 MODULE EigenUtils
        URL http://eigen.tuxfamily.org/
        TYPE "Mozilla Public License Version 2.0"
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/eigengitlab/COPYING.README
              ${CMAKE_CURRENT_SOURCE_DIR}/ext/eigengitlab/COPYING.MPL2
    ) 
else()
    find_package(Eigen3 REQUIRED)
    target_link_libraries(inviwo-module-eigenutils PUBLIC Eigen3::Eigen)
    ivw_vcpkg_install(eigen3 MODULE EigenUtils)
endif()
