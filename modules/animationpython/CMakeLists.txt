# Inviwo AnimationPython Module
ivw_module(AnimationPython)

# Add header files
set(HEADER_FILES
    include/inviwo/animationpython/animationpythonmodule.h
    include/inviwo/animationpython/animationpythonmoduledefine.h
)
ivw_group("Header Files" ${HEADER_FILES})

# Add source files
set(SOURCE_FILES
    src/animationpythonmodule.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Add Unittests
set(TEST_FILES
    tests/unittests/animationpython-unittest-main.cpp
    tests/unittests/animation-test.cpp
)
ivw_add_unittest(${TEST_FILES})

# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})

find_package(pybind11 CONFIG REQUIRED)
target_link_libraries(inviwo-module-animationpython PUBLIC
    pybind11::embed
    Python3::Python
)

add_subdirectory(bindings)

if(TARGET inviwo-unittests-animationpython)
    add_dependencies(inviwo-unittests-animationpython ivwanimation inviwopy)
    target_link_libraries(inviwo-unittests-animationpython PUBLIC
        inviwo::python-helper
        inviwo-module-base
    )
endif()
