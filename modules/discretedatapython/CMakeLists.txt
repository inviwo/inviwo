#--------------------------------------------------------------------
# Inviwo DiscreteDataPython Module
ivw_module(DiscreteDataPython)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/modules/discretedatapython/channels/numpychannel.h
    include/modules/discretedatapython/discretedatapythonmodule.h
    include/modules/discretedatapython/discretedatapythonmoduledefine.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/channels/numpychannel.cpp
    src/discretedatapythonmodule.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
    tests/unittests/discretedatapython-unittest-main.cpp
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})

find_package(pybind11 CONFIG REQUIRED)
target_link_libraries(inviwo-module-discretedatapython PUBLIC 
    pybind11::embed 
    $<BUILD_INTERFACE:${PYTHON_LIBRARIES}>
)

add_subdirectory(bindings)

if(TARGET inviwo-unittests-discretedatapython)
    add_dependencies(inviwo-unittests-discretedatapython ivwdiscretedata inviwopy)
endif()
