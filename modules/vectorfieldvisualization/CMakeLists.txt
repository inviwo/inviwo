#--------------------------------------------------------------------
# Inviwo VectorFieldVisualization Module
ivw_module(VectorFieldVisualization)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/modules/vectorfieldvisualization/algorithms/integrallineoperations.h
    include/modules/vectorfieldvisualization/datastructures/integralline.h
    include/modules/vectorfieldvisualization/datastructures/integrallineset.h
    include/modules/vectorfieldvisualization/generatestreamfunction.h
    include/modules/vectorfieldvisualization/integrallinetracer.h
    include/modules/vectorfieldvisualization/ports/seedpointsport.h
    include/modules/vectorfieldvisualization/processors/2d/seedpointgenerator2d.h
    include/modules/vectorfieldvisualization/processors/2d/seedstomesh2d.h
    include/modules/vectorfieldvisualization/processors/3d/pathlines.h
    include/modules/vectorfieldvisualization/processors/3d/streamlines.h
    include/modules/vectorfieldvisualization/processors/3d/streamribbons.h
    include/modules/vectorfieldvisualization/processors/addvalueperline.h
    include/modules/vectorfieldvisualization/processors/arrowglyphs.h
    include/modules/vectorfieldvisualization/processors/currents/highvelocitylineregions.h
    include/modules/vectorfieldvisualization/processors/currents/pathsbetweenregions.h
    include/modules/vectorfieldvisualization/processors/currents/showstreampath.h
    include/modules/vectorfieldvisualization/processors/currents/streamspanningtree.h
    include/modules/vectorfieldvisualization/processors/currents/streamspanningtree3d.h
    include/modules/vectorfieldvisualization/processors/currents/streamspanningtreejit.h
    include/modules/vectorfieldvisualization/processors/datageneration/holgersthreelines.h
    include/modules/vectorfieldvisualization/processors/datageneration/rbfvectorfieldgenerator2d.h
    include/modules/vectorfieldvisualization/processors/datageneration/rbfvectorfieldgenerator3d.h
    include/modules/vectorfieldvisualization/processors/datageneration/seedpointgenerator.h
    include/modules/vectorfieldvisualization/processors/datageneration/seedpointsfrommask.h
    include/modules/vectorfieldvisualization/processors/datageneration/seedpointsfrommask2d.h
    include/modules/vectorfieldvisualization/processors/discardshortlines.h
    include/modules/vectorfieldvisualization/processors/flowfieldprocessor.h
    include/modules/vectorfieldvisualization/processors/integrallineannotation.h
    include/modules/vectorfieldvisualization/processors/integrallinestocomets.h
    include/modules/vectorfieldvisualization/processors/integrallinetracerprocessor.h
    include/modules/vectorfieldvisualization/processors/integrallinevectortomesh.h
    include/modules/vectorfieldvisualization/processors/linesfromdataframe.h
    include/modules/vectorfieldvisualization/processors/linesourceascii.h
    include/modules/vectorfieldvisualization/processors/linestoarrows.h
    include/modules/vectorfieldvisualization/processors/meshtocomets.h
    include/modules/vectorfieldvisualization/processors/meshtointegrallines.h
    include/modules/vectorfieldvisualization/processors/perpendicularsimilarity.h
    include/modules/vectorfieldvisualization/processors/scalarconvolutionprocessor.h
    include/modules/vectorfieldvisualization/processors/seed3dto4d.h
    include/modules/vectorfieldvisualization/processors/seedsfrommasksequence.h
    include/modules/vectorfieldvisualization/processors/seedsfrommesh.h
    include/modules/vectorfieldvisualization/processors/streamlinepathways.h
    include/modules/vectorfieldvisualization/processors/unsteadyvolumetospatialsampler.h
    include/modules/vectorfieldvisualization/properties/integrallineproperties.h
    include/modules/vectorfieldvisualization/properties/pathlineproperties.h
    include/modules/vectorfieldvisualization/properties/streamlineproperties.h
    include/modules/vectorfieldvisualization/util/linemetrics.h
    include/modules/vectorfieldvisualization/util/unsteadyspatialsampler.h
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
    src/generatestreamfunction.cpp
    src/integrallinetracer.cpp
    src/processors/2d/seedpointgenerator2d.cpp
    src/processors/2d/seedstomesh2d.cpp
    src/processors/3d/pathlines.cpp
    src/processors/3d/streamlines.cpp
    src/processors/3d/streamribbons.cpp
    src/processors/addvalueperline.cpp
    src/processors/arrowglyphs.cpp
    src/processors/currents/highvelocitylineregions.cpp
    src/processors/currents/pathsbetweenregions.cpp
    src/processors/currents/showstreampath.cpp
    src/processors/currents/streamspanningtree.cpp
    src/processors/currents/streamspanningtree3d.cpp
    src/processors/currents/streamspanningtreejit.cpp
    src/processors/datageneration/holgersthreelines.cpp
    src/processors/datageneration/rbfvectorfieldgenerator2d.cpp
    src/processors/datageneration/rbfvectorfieldgenerator3d.cpp
    src/processors/datageneration/seedpointgenerator.cpp
    src/processors/datageneration/seedpointsfrommask.cpp
    src/processors/datageneration/seedpointsfrommask2d.cpp
    src/processors/discardshortlines.cpp
    src/processors/flowfieldprocessor.cpp
    src/processors/integrallineannotation.cpp
    src/processors/integrallinestocomets.cpp
    src/processors/integrallinevectortomesh.cpp
    src/processors/linesfromdataframe.cpp
    src/processors/linesourceascii.cpp
    src/processors/linestoarrows.cpp
    src/processors/meshtocomets.cpp
    src/processors/meshtointegrallines.cpp
    src/processors/perpendicularsimilarity.cpp
    src/processors/scalarconvolutionprocessor.cpp
    src/processors/seed3dto4d.cpp
    src/processors/seedsfrommasksequence.cpp
    src/processors/seedsfrommesh.cpp
    src/processors/streamlinepathways.cpp
    src/processors/unsteadyvolumetospatialsampler.cpp
    src/properties/integrallineproperties.cpp
    src/properties/pathlineproperties.cpp
    src/properties/streamlineproperties.cpp
    src/util/linemetrics.cpp
    src/util/unsteadyspatialsampler.cpp
    src/vectorfieldvisualizationmodule.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})
