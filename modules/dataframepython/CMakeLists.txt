# Inviwo DataFramePython Module
ivw_module(DataFramePython)

# Add header files
set(HEADER_FILES
    include/inviwo/dataframepython/dataframepythonmodule.h
    include/inviwo/dataframepython/dataframepythonmoduledefine.h
)
ivw_group("Header Files" ${HEADER_FILES})

# Add source files
set(SOURCE_FILES
    src/dataframepythonmodule.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Add Unittests
set(TEST_FILES
    tests/unittests/dataframepython-unittest-main.cpp
    tests/unittests/column-test.cpp
    tests/unittests/dataframe-test.cpp
)
ivw_add_unittest(${TEST_FILES})

# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})

find_package(pybind11 CONFIG REQUIRED)
target_link_libraries(inviwo-module-dataframepython PUBLIC 
    pybind11::embed 
    Python3::Python
)

add_subdirectory(bindings)

if(TARGET inviwo-unittests-dataframepython)
    add_dependencies(inviwo-unittests-dataframepython ivwdataframe inviwopy)
    target_link_libraries(inviwo-unittests-dataframepython PUBLIC inviwo::python-helper)
endif()
