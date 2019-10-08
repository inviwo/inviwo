#--------------------------------------------------------------------
# Inviwo discrete data Module
ivw_module(DiscreteData)

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
    include/modules/discretedata/channels/datachannel.h
    include/modules/discretedata/connectivity/cell.h
    include/modules/discretedata/connectivity/connectioniterator.h
    include/modules/discretedata/connectivity/connectivity.h
    include/modules/discretedata/connectivity/elementiterator.h
    include/modules/discretedata/connectivity/euclideanmeasure.h
    include/modules/discretedata/connectivity/lineset.h
    include/modules/discretedata/connectivity/periodicgrid.h
    include/modules/discretedata/connectivity/structuredgrid.h
    include/modules/discretedata/ports/datasetport.h
    include/modules/discretedata/processors/computegridmeasure.h
    include/modules/discretedata/processors/createchannel.h
    include/modules/discretedata/processors/datasetfrombase.h
    include/modules/discretedata/processors/datasetsource.h
    include/modules/discretedata/processors/meshfromdataset.h
    include/modules/discretedata/processors/segmentationvoxelizer.h
    include/modules/discretedata/processors/volumefromdataset.h
    include/modules/discretedata/properties/datachannelproperty.h
    include/modules/discretedata/dataset.h
    include/modules/discretedata/discretedatamodule.h
    include/modules/discretedata/discretedatamoduledefine.h
    include/modules/discretedata/discretedatatypes.h
    include/modules/discretedata/util.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/channels/buffergetter.cpp
    src/channels/cachedgetter.cpp
    src/channels/channel.cpp
    src/channels/datachannel.cpp
    src/connectivity/connectioniterator.cpp
    src/connectivity/connectivity.cpp
    src/connectivity/elementiterator.cpp
    src/connectivity/euclideanmeasure.cpp
    src/connectivity/lineset.cpp
    src/processors/computegridmeasure.cpp
    src/processors/createchannel.cpp
    src/processors/datasetfrombase.cpp
    src/processors/datasetsource.cpp
    src/processors/meshfromdataset.cpp
    src/processors/segmentationvoxelizer.cpp
    src/processors/volumefromdataset.cpp
    src/properties/datachannelproperty.cpp
    src/dataset.cpp
    src/discretedatamodule.cpp
    src/discretedatatypes.cpp
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
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(NO_PCH ${SOURCE_FILES} ${HEADER_FILES})
