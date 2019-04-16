#--------------------------------------------------------------------
# Inviwo DataFrame Module
ivw_module(DataFrame)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
	include/inviwo/dataframe/dataframemodule.h
	include/inviwo/dataframe/dataframemoduledefine.h
	include/inviwo/dataframe/jsondataframeconversion.h
	include/inviwo/dataframe/datastructures/column.h
	include/inviwo/dataframe/datastructures/dataframe.h
	include/inviwo/dataframe/datastructures/dataframeutil.h
	include/inviwo/dataframe/datastructures/datapoint.h
	include/inviwo/dataframe/io/csvreader.h
	include/inviwo/dataframe/io/jsonreader.h
	include/inviwo/dataframe/processors/csvsource.h
	include/inviwo/dataframe/processors/dataframesource.h
	include/inviwo/dataframe/processors/dataframeexporter.h
	include/inviwo/dataframe/processors/imagetodataframe.h
	include/inviwo/dataframe/processors/syntheticdataframe.h
	include/inviwo/dataframe/processors/volumesequencetodataframe.h
	include/inviwo/dataframe/processors/volumetodataframe.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
	src/dataframemodule.cpp
	src/jsondataframeconversion.cpp
	src/datastructures/column.cpp
	src/datastructures/dataframe.cpp
	src/datastructures/dataframeutil.cpp
	src/io/csvreader.cpp
	src/io/jsonreader.cpp
	src/processors/csvsource.cpp
	src/processors/dataframeexporter.cpp
	src/processors/dataframesource.cpp
	src/processors/imagetodataframe.cpp
	src/processors/syntheticdataframe.cpp
	src/processors/volumesequencetodataframe.cpp
	src/processors/volumetodataframe.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
)
ivw_group("Shader Files" ${SHADER_FILES})

#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
	tests/unittests/dataframe-unittest-main.cpp
	tests/unittests/jsonreader-test.cpp
	tests/unittests/csvreader-test.cpp
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

target_link_libraries(inviwo-module-dataframe PUBLIC
	nlohmann_json::nlohmann_json
)

#--------------------------------------------------------------------
# Register license for external library
ivw_register_license_file(NAME "JSON for Modern C++" target nlohmann_json MODULE DataFrame
	URL "https://github.com/nlohmann/json"
	TYPE "The MIT License"
	FILES  ${IVW_EXTENSIONS_DIR}/json/LICENSE.MIT
)
