#--------------------------------------------------------------------
# Inviwo Base OpenCL Module
ivw_module(BaseCL)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/modules/basecl/baseclmodule.h
    include/modules/basecl/baseclmoduledefine.h
    include/modules/basecl/meshentryexitpointscl.h
    include/modules/basecl/processors/entryexitpointsclprocessor.h
    include/modules/basecl/processors/grayscaleclprocessor.h
    include/modules/basecl/processors/volumefirsthitclprocessor.h
    include/modules/basecl/processors/volumemaxclprocessor.h
    include/modules/basecl/processors/volumeraycasterclprocessor.h
    include/modules/basecl/runningimagemeanandstandarddeviationcl.h
    include/modules/basecl/volumeraycastercl.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/baseclmodule.cpp
    src/meshentryexitpointscl.cpp
    src/processors/entryexitpointsclprocessor.cpp
    src/processors/grayscaleclprocessor.cpp
    src/processors/volumefirsthitclprocessor.cpp
    src/processors/volumemaxclprocessor.cpp
    src/processors/volumeraycasterclprocessor.cpp
    src/runningimagemeanandstandarddeviationcl.cpp
    src/volumeraycastercl.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Add OpenCL files
set(SHADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/entryexitpoints.cl
	${CMAKE_CURRENT_SOURCE_DIR}/cl/intersection/raymeshintersection.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/grayscale.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/statistics/runningmeanandstandarddeviation.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/statistics/runningmeanandstandarddeviationkernel.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/volumefirsthit.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/volumemax.cl
    ${CMAKE_CURRENT_SOURCE_DIR}/cl/volumeraycaster.cl
)
ivw_group("Shader Files" ${SHADER_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

#--------------------------------------------------------------------
# Add kernel directory to pack
ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/cl)
