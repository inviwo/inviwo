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
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

target_include_directories(inviwo-module-eigenutils PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ext/eigen>
)

ivw_register_license_file(ID eigen NAME Eigen VERSION 0.0.0.0 MODULE EigenUtils
    URL http://eigen.tuxfamily.org/
    FILES 
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/eigen/COPYING.README
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/eigen/COPYING.MPL2
)
ivw_make_package(InviwoEigenUtilsModule inviwo-module-eigenutils)