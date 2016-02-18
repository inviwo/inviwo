#--------------------------------------------------------------------
# Inviwo VectorFieldVisualization Module
ivw_module(VectorFieldVisualization)


#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/integralline.h
    ${CMAKE_CURRENT_SOURCE_DIR}/integrallinetracer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pathlinetracer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ports/seedpointsport.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/3d/pathlines.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/3d/streamlines.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/3d/streamribbons.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/datageneration/rbfvectorfieldgenerator2d.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/datageneration/rbfvectorfieldgenerator3d.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/datageneration/seedpointgenerator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/integrallineproperties.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/pathlineproperties.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/streamlineproperties.h
    ${CMAKE_CURRENT_SOURCE_DIR}/streamlinetracer.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/integralline.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/integrallinetracer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pathlinetracer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/3d/pathlines.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/3d/streamlines.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/3d/streamribbons.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/datageneration/rbfvectorfieldgenerator2d.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/datageneration/rbfvectorfieldgenerator3d.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/datageneration/seedpointgenerator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/integrallineproperties.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/pathlineproperties.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/streamlineproperties.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/streamlinetracer.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})