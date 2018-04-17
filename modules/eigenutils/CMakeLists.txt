#--------------------------------------------------------------------
# Inviwo EigenUtils Module
ivw_module(EigenUtils)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/eigenutils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/eigenports.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/eigenmatrixtoimage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/eigenmix.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/eigennormalize.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/eigenmatrixtoimage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/eigenmix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/eigennormalize.cpp
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