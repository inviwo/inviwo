#--------------------------------------------------------------------
# Inviwo DataFramePython Module
ivw_module(DataFramePython)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/inviwo/dataframepython/dataframepythonmodule.h
    include/inviwo/dataframepython/dataframepythonmoduledefine.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/dataframepythonmodule.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
    tests/unittests/dataframepython-unittest-main.cpp
    tests/unittests/column-test.cpp
    tests/unittests/dataframe-test.cpp
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(inviwo-module-dataframepython PUBLIC 
    pybind11::embed 
    $<BUILD_INTERFACE:${PYTHON_LIBRARIES}>
)

add_subdirectory(bindings)

if(IVW_UNITTESTS)
    add_dependencies(inviwo-unittests-dataframepython ivwdataframe)
endif()
