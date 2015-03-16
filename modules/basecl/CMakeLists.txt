#--------------------------------------------------------------------
# Inviwo Base OpenCL Module
ivw_module(BaseCL)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/meshentryexitpointscl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/entryexitpointsclprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/grayscaleclprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumefirsthitclprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumemaxclprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeraycasterclprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/runningimagemeanandstandarddeviationcl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/volumeraycastercl.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/meshentryexitpointscl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/entryexitpointsclprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/grayscaleclprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumefirsthitclprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumemaxclprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeraycasterclprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/runningimagemeanandstandarddeviationcl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/volumeraycastercl.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Add OpenCL files
set(SHADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/entryexitpoints.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/grayscale.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/statistics/runningmeanandstandarddeviation.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/statistics/runningmeanandstandarddeviationkernel.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/volumefirsthit.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/volumemax.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/volumeraycaster.cl
)
ivw_group("Shader Files" ${SHADER_FILES})

#set(TEST_FILES
#    ${CMAKE_CURRENT_SOURCE_DIR}/tests/runningimagemeanandstandarddeviationcl-test.cpp
#)
#ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Add definition
ivw_add_definition(__CL_ENABLE_EXCEPTIONS)

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

#--------------------------------------------------------------------
# Add kernel directory to pack
ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/cl)