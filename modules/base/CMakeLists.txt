#--------------------------------------------------------------------
# Inviwo Base Module
ivw_module(Base)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/cubeproxygeometry.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/datasource.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/diffuselightsourceprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/directionallightsourceprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/distancetransformram.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/geometrysource.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/heightfieldmapper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imageexport.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imagesource.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imagesourceseries.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/meshclipping.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/meshcreator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/ordinalpropertyanimator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/pixeltobufferprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/pointlightsourceprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/spotlightsourceprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumebasistransformer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeexport.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeslice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumesource.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumesubsample.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumesubset.h
)
ivw_group("Header Files" ${HEADER_FILES})


#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/cubeproxygeometry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/diffuselightsourceprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/directionallightsourceprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/distancetransformram.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/geometrysource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/heightfieldmapper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imageexport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imagesource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/imagesourceseries.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/meshclipping.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/meshcreator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/ordinalpropertyanimator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/pixeltobufferprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/pointlightsourceprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/spotlightsourceprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumebasistransformer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeexport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumeslice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumesource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumesubsample.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/volumesubset.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${MOC_FILES} ${HEADER_FILES})