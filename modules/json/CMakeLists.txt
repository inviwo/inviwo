# Inviwo json Module
ivw_module(JSON)

set(HEADER_FILES
    include/modules/json/io/json/boolpropertyjsonconverter.h
    include/modules/json/io/json/buttonpropertyjsonconverter.h
    include/modules/json/io/json/directorypropertyjsonconverter.h
    include/modules/json/io/json/filepropertyjsonconverter.h
    include/modules/json/io/json/glmjsonconverter.h
    include/modules/json/io/json/minmaxpropertyjsonconverter.h
    include/modules/json/io/json/optionpropertyjsonconverter.h
    include/modules/json/io/json/ordinalpropertyjsonconverter.h
    include/modules/json/io/json/ordinalrefpropertyjsonconverter.h
    include/modules/json/io/json/stringpropertyjsonconverter.h
    include/modules/json/io/json/templatepropertyjsonconverter.h
    include/modules/json/io/jsonreader.h
    include/modules/json/io/jsonwriter.h
    include/modules/json/json.h
    include/modules/json/jsonconverter.h
    include/modules/json/jsonconverterregistry.h
    include/modules/json/jsoninportconverter.h
    include/modules/json/jsonmodule.h
    include/modules/json/jsonmoduledefine.h
    include/modules/json/jsonoutportconverter.h
    include/modules/json/jsonport.h
    include/modules/json/jsonpropertyconverter.h
    include/modules/json/jsonsupplier.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/io/json/boolpropertyjsonconverter.cpp
    src/io/json/buttonpropertyjsonconverter.cpp
    src/io/json/directorypropertyjsonconverter.cpp
    src/io/json/filepropertyjsonconverter.cpp
    src/io/json/ordinalrefpropertyjsonconverter.cpp
    src/io/json/stringpropertyjsonconverter.cpp
    src/io/jsonreader.cpp
    src/io/jsonwriter.cpp
    src/json.cpp
    src/jsonconverter.cpp
    src/jsonconverterregistry.cpp
    src/jsoninportconverter.cpp
    src/jsonmodule.cpp
    src/jsonoutportconverter.cpp
    src/jsonport.cpp
    src/jsonpropertyconverter.cpp
    src/jsonsupplier.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Add Unittests
set(TEST_FILES
	#${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/json-test.cpp
)
ivw_add_unittest(${TEST_FILES})

# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(inviwo-module-json PUBLIC
    nlohmann_json::nlohmann_json
)
