#--------------------------------------------------------------------
# Inviwo discrete data Module
ivw_module(DiscreteData)

SET(IVW_DISCRETEDATA_MAX_NUM_DIMENSIONS 4 CACHE INTEGER "The default maximum number of dimensions compiled for")
add_definitions( -DDISCRETEDATA_MAX_NUM_DIMENSIONS=${IVW_DISCRETEDATA_MAX_NUM_DIMENSIONS} )

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/modules/discretedata/channels/analyticchannel.h
    include/modules/discretedata/channels/bufferchannel.h
    include/modules/discretedata/channels/buffergetter.h
    include/modules/discretedata/channels/cachedgetter.h
    include/modules/discretedata/channels/channel.h
    include/modules/discretedata/channels/channelgetter.h
    include/modules/discretedata/channels/channeliterator.h
    include/modules/discretedata/channels/combinedchannel.h
    include/modules/discretedata/channels/datachannel.h
    include/modules/discretedata/connectivity/cell.h
    include/modules/discretedata/connectivity/connectioniterator.h
    include/modules/discretedata/connectivity/connectivity.h
    include/modules/discretedata/connectivity/elementiterator.h
    include/modules/discretedata/connectivity/euclideanmeasure.h
    include/modules/discretedata/connectivity/explicitconnectivity.h
    include/modules/discretedata/connectivity/lineset.h
    include/modules/discretedata/connectivity/periodicgrid.h
    include/modules/discretedata/connectivity/structuredgrid.h
    include/modules/discretedata/connectivity/trianglemesh.h
    include/modules/discretedata/connectivity/tripolargrid.h
    include/modules/discretedata/dataset.h
    include/modules/discretedata/discretedatamodule.h
    include/modules/discretedata/discretedatamoduledefine.h
    include/modules/discretedata/discretedatatypes.h
    include/modules/discretedata/interpolation/interpolant.h
    include/modules/discretedata/ports/datasetport.h
    include/modules/discretedata/processors/adddatasetsampler.h
    include/modules/discretedata/processors/colormapchannel.h
    include/modules/discretedata/processors/combinechannels.h
    include/modules/discretedata/processors/computegridmeasure.h
    include/modules/discretedata/processors/createconstantchannel.h
    include/modules/discretedata/processors/createindexchannel.h
    include/modules/discretedata/processors/createuniformgrid.h
    include/modules/discretedata/processors/datasetfrombase.h
    include/modules/discretedata/processors/datasetsource.h
    include/modules/discretedata/processors/datasettospatialsampler.h
    include/modules/discretedata/processors/exampledataset.h
    include/modules/discretedata/processors/meshfromdataset.h
    include/modules/discretedata/processors/segmentationvoxelizer.h
    include/modules/discretedata/processors/sphericalcoordinates.h
    include/modules/discretedata/processors/volumefromdataset.h
    include/modules/discretedata/properties/datachannelproperty.h
    include/modules/discretedata/sampling/celltree.h
    include/modules/discretedata/sampling/datasetspatialsampler.h
    include/modules/discretedata/util/prefixsumvector.h
    include/modules/discretedata/util/spatialentitychannel.h
    include/modules/discretedata/util/util.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/channels/buffergetter.cpp
    src/channels/cachedgetter.cpp
    src/channels/channel.cpp
    src/channels/combinedchannel.cpp
    src/channels/datachannel.cpp
    src/connectivity/cell.cpp
    src/connectivity/connectioniterator.cpp
    src/connectivity/connectivity.cpp
    src/connectivity/elementiterator.cpp
    src/connectivity/euclideanmeasure.cpp
    src/connectivity/explicitconnectivity.cpp
    src/connectivity/lineset.cpp
    src/connectivity/trianglemesh.cpp
    src/dataset.cpp
    src/discretedatamodule.cpp
    src/discretedatatypes.cpp
    src/processors/adddatasetsampler.cpp
    src/processors/colormapchannel.cpp
    src/processors/combinechannels.cpp
    src/processors/computegridmeasure.cpp
    src/processors/createconstantchannel.cpp
    src/processors/createindexchannel.cpp
    src/processors/createuniformgrid.cpp
    src/processors/datasetfrombase.cpp
    src/processors/datasetsource.cpp
    src/processors/exampledataset.cpp
    src/processors/meshfromdataset.cpp
    src/processors/segmentationvoxelizer.cpp
    src/processors/sphericalcoordinates.cpp
    src/processors/volumefromdataset.cpp
    src/properties/datachannelproperty.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/data-unittest-main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/data-test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/dataset-test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/data-access-test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/example-code.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/lineset-test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/structured-grid-test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/tripolar-grid-test.cpp
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(NO_PCH ${SOURCE_FILES} ${HEADER_FILES})
