#--------------------------------------------------------------------
# Inviwo Base Module
ivw_module(Base)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/algorithm/image/imagecontour.h
    ${CMAKE_CURRENT_SOURCE_DIR}/algorithm/volume/marchingtetrahedron.h
    ${CMAKE_CURRENT_SOURCE_DIR}/algorithm/volume/volumeramsubsample.h
    ${CMAKE_CURRENT_SOURCE_DIR}/algorithm/volume/volumeramsubset.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/kdtree.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/cubeproxygeometry.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/datasource.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/diffuselightsourceprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/directionallightsourceprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/distancetransformram.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/heightfieldmapper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imagecontourprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imageexport.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imagesnapshot.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imagesource.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imagesourceseries.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/meshclipping.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/meshcreator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/meshsource.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/noiseprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/ordinalpropertyanimator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/pixeltobufferprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/pointlightsourceprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/spotlightsourceprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/stereocamerasyncer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/surfaceextraction.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/timestepselector.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumebasistransformer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeexport.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeslice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumesource.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumesubsample.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumesubset.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumevectorsource.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/basisproperty.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/gaussianproperty.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/sequencetimerproperty.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/volumeinformationproperty.h
)
ivw_group("Header Files" ${HEADER_FILES})


#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/algorithm/image/imagecontour.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/algorithm/volume/marchingtetrahedron.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/algorithm/volume/volumeramsubsample.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/algorithm/volume/volumeramsubset.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/cubeproxygeometry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/diffuselightsourceprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/directionallightsourceprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/distancetransformram.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/heightfieldmapper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imagecontourprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imageexport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imagesnapshot.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imagesource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imagesourceseries.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/meshclipping.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/meshcreator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/meshsource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/noiseprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/ordinalpropertyanimator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/pixeltobufferprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/pointlightsourceprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/spotlightsourceprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/stereocamerasyncer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/surfaceextraction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/timestepselector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeexport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeslice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumesource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumesubsample.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumesubset.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumevectorsource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/basisproperty.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/gaussianproperty.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/sequencetimerproperty.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/volumeinformationproperty.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Unit tests
set(TEST_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/base-unittest-main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/kdtree-test.cpp
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${MOC_FILES} ${HEADER_FILES})