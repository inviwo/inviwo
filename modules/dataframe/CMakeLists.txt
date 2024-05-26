# Inviwo DataFrame Module
ivw_module(DataFrame)

# Add header files
set(HEADER_FILES
    include/inviwo/dataframe/dataframemodule.h
    include/inviwo/dataframe/dataframemoduledefine.h
    include/inviwo/dataframe/datastructures/column.h
    include/inviwo/dataframe/datastructures/dataframe.h
    include/inviwo/dataframe/io/csvreader.h
    include/inviwo/dataframe/io/csvwriter.h
    include/inviwo/dataframe/io/json/dataframepropertyjsonconverter.h
    include/inviwo/dataframe/io/jsonreader.h
    include/inviwo/dataframe/io/xmlwriter.h
    include/inviwo/dataframe/jsondataframeconversion.h
    include/inviwo/dataframe/processors/csvsource.h
    include/inviwo/dataframe/processors/dataframeexporter.h
    include/inviwo/dataframe/processors/dataframefilter.h
    include/inviwo/dataframe/processors/dataframefloat32converter.h
    include/inviwo/dataframe/processors/dataframejoin.h
    include/inviwo/dataframe/processors/dataframemetadata.h
    include/inviwo/dataframe/processors/dataframesource.h
    include/inviwo/dataframe/processors/dataframetovector.h
    include/inviwo/dataframe/processors/imagetodataframe.h
    include/inviwo/dataframe/processors/syntheticdataframe.h
    include/inviwo/dataframe/processors/volumesequencetodataframe.h
    include/inviwo/dataframe/processors/volumetodataframe.h
    include/inviwo/dataframe/properties/colormapproperty.h
    include/inviwo/dataframe/properties/columnmetadatalistproperty.h
    include/inviwo/dataframe/properties/columnmetadataproperty.h
    include/inviwo/dataframe/properties/columnoptionproperty.h
    include/inviwo/dataframe/properties/dataframecolormapproperty.h
    include/inviwo/dataframe/properties/filterlistproperty.h
    include/inviwo/dataframe/properties/optionconverter.h
    include/inviwo/dataframe/util/dataframeutil.h
    include/inviwo/dataframe/util/filters.h
)
ivw_group("Header Files" ${HEADER_FILES})

# Add source files
set(SOURCE_FILES
    src/dataframemodule.cpp
    src/datastructures/column.cpp
    src/datastructures/dataframe.cpp
    src/io/csvreader.cpp
    src/io/csvwriter.cpp
    src/io/json/dataframepropertyjsonconverter.cpp
    src/io/jsonreader.cpp
    src/io/xmlwriter.cpp
    src/jsondataframeconversion.cpp
    src/processors/csvsource.cpp
    src/processors/dataframeexporter.cpp
    src/processors/dataframefilter.cpp
    src/processors/dataframefloat32converter.cpp
    src/processors/dataframejoin.cpp
    src/processors/dataframemetadata.cpp
    src/processors/dataframesource.cpp
    src/processors/dataframetovector.cpp
    src/processors/imagetodataframe.cpp
    src/processors/syntheticdataframe.cpp
    src/processors/volumesequencetodataframe.cpp
    src/processors/volumetodataframe.cpp
    src/properties/colormapproperty.cpp
    src/properties/columnmetadatalistproperty.cpp
    src/properties/columnmetadataproperty.cpp
    src/properties/columnoptionproperty.cpp
    src/properties/dataframecolormapproperty.cpp
    src/properties/filterlistproperty.cpp
    src/properties/optionconverter.cpp
    src/util/dataframeutil.cpp
    src/util/filters.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Add shaders
set(SHADER_FILES
)
ivw_group("Shader Files" ${SHADER_FILES})

# Add Unittests
set(TEST_FILES
    tests/unittests/column-test.cpp
    tests/unittests/csvreader-test.cpp
    tests/unittests/dataframe-test.cpp
    tests/unittests/dataframe-unittest-main.cpp
    tests/unittests/join-test.cpp
    tests/unittests/jsonreader-test.cpp
)
ivw_add_unittest(${TEST_FILES})

if(IVW_TEST_BENCHMARKS)
    add_subdirectory(tests/benchmarks)
endif()

# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})
