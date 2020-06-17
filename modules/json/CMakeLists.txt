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
    include/modules/json/io/json/propertyjsonconverter.h
    include/modules/json/io/json/propertyjsonconverterfactory.h
    include/modules/json/io/json/propertyjsonconverterfactoryobject.h
    include/modules/json/io/json/templatepropertyjsonconverter.h
    include/modules/json/jsonmodule.h
    include/modules/json/jsonmoduledefine.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/io/json/boolpropertyjsonconverter.cpp
    src/io/json/buttonpropertyjsonconverter.cpp
    src/io/json/directorypropertyjsonconverter.cpp
    src/io/json/filepropertyjsonconverter.cpp
    src/io/json/ordinalrefpropertyjsonconverter.cpp
    src/io/json/propertyjsonconverterfactory.cpp
    src/io/json/propertyjsonconverterfactoryobject.cpp
    src/jsonmodule.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Add Unittests
set(TEST_FILES
	#${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/json-test.cpp
)
ivw_add_unittest(${TEST_FILES})

# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})
target_link_libraries(inviwo-module-json PUBLIC
    nlohmann_json::nlohmann_json
)

# Register license for external library
ivw_register_license_file(NAME "JSON for Modern C++" target nlohmann_json MODULE JSON
URL "https://github.com/nlohmann/json"
TYPE "The MIT License"
FILES  ${IVW_EXTENSIONS_DIR}/json/LICENSE.MIT
)
