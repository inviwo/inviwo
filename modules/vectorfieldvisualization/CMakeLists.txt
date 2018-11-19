#--------------------------------------------------------------------
# Inviwo VectorFieldVisualization Module
ivw_module(VectorFieldVisualization)


#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/modules/vectorfieldvisualization/algorithms/integrallineoperations.h
    include/modules/vectorfieldvisualization/datastructures/integralline.h
    include/modules/vectorfieldvisualization/datastructures/integrallineset.h
    include/modules/vectorfieldvisualization/integrallinetracer.h
    include/modules/vectorfieldvisualization/ports/seedpointsport.h
    include/modules/vectorfieldvisualization/processors/2d/seedpointgenerator2d.h
    include/modules/vectorfieldvisualization/processors/3d/pathlines.h
    include/modules/vectorfieldvisualization/processors/3d/streamlines.h
    include/modules/vectorfieldvisualization/processors/3d/streamribbons.h
    include/modules/vectorfieldvisualization/processors/datageneration/rbfvectorfieldgenerator2d.h
    include/modules/vectorfieldvisualization/processors/datageneration/rbfvectorfieldgenerator3d.h
    include/modules/vectorfieldvisualization/processors/datageneration/seedpointgenerator.h
    include/modules/vectorfieldvisualization/processors/datageneration/seedpointsfrommask.h
    include/modules/vectorfieldvisualization/processors/discardshortlines.h
    include/modules/vectorfieldvisualization/processors/integrallinetracerprocessor.h
    include/modules/vectorfieldvisualization/processors/integrallinevectortomesh.h
    include/modules/vectorfieldvisualization/processors/seed3dto4d.h
    include/modules/vectorfieldvisualization/processors/seedsfrommasksequence.h
    include/modules/vectorfieldvisualization/properties/integrallineproperties.h
    include/modules/vectorfieldvisualization/properties/pathlineproperties.h
    include/modules/vectorfieldvisualization/properties/streamlineproperties.h
    include/modules/vectorfieldvisualization/vectorfieldvisualizationmodule.h
    include/modules/vectorfieldvisualization/vectorfieldvisualizationmoduledefine.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/algorithms/integrallineoperations.cpp
    src/datastructures/integralline.cpp
    src/datastructures/integrallineset.cpp
    src/integrallinetracer.cpp
    src/processors/2d/seedpointgenerator2d.cpp
    src/processors/3d/pathlines.cpp
    src/processors/3d/streamlines.cpp
    src/processors/3d/streamribbons.cpp
    src/processors/datageneration/rbfvectorfieldgenerator2d.cpp
    src/processors/datageneration/rbfvectorfieldgenerator3d.cpp
    src/processors/datageneration/seedpointgenerator.cpp
    src/processors/datageneration/seedpointsfrommask.cpp
    src/processors/discardshortlines.cpp
    src/processors/integrallinevectortomesh.cpp
    src/processors/seed3dto4d.cpp
    src/processors/seedsfrommasksequence.cpp
    src/properties/integrallineproperties.cpp
    src/properties/pathlineproperties.cpp
    src/properties/streamlineproperties.cpp
    src/vectorfieldvisualizationmodule.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})
