# Inviwo Plotting Module
ivw_module(Plotting)

# Add header files
set(HEADER_FILES
    include/modules/plotting/algorithm/labeling.h
    include/modules/plotting/datastructures/axisdata.h
    include/modules/plotting/datastructures/boxselection.h
    include/modules/plotting/datastructures/tickdata.h
    include/modules/plotting/datastructures/textdata.h
    include/modules/plotting/interaction/boxselectioninteractionhandler.h
    include/modules/plotting/plottingmodule.h
    include/modules/plotting/plottingmoduledefine.h
    include/modules/plotting/processors/dataframecolumntocolorvector.h
    include/modules/plotting/properties/axisproperty.h
    include/modules/plotting/properties/axisstyleproperty.h
    include/modules/plotting/properties/boxselectionproperty.h
    include/modules/plotting/properties/categoricalaxisproperty.h
    include/modules/plotting/properties/plottextproperty.h
    include/modules/plotting/properties/tickproperty.h
    include/modules/plotting/utils/axisutils.h
    include/modules/plotting/utils/statsutils.h
)
ivw_group("Header Files" ${HEADER_FILES})

# Add source files
set(SOURCE_FILES
    src/algorithm/labeling.cpp
    src/datastructures/axisdata.cpp
    src/datastructures/boxselection.cpp
    src/datastructures/tickdata.cpp
    src/datastructures/textdata.cpp
    src/interaction/boxselectioninteractionhandler.cpp
    src/plottingmodule.cpp
    src/processors/dataframecolumntocolorvector.cpp
    src/properties/axisproperty.cpp
    src/properties/axisstyleproperty.cpp
    src/properties/boxselectionproperty.cpp
    src/properties/categoricalaxisproperty.cpp
    src/properties/plottextproperty.cpp
    src/properties/tickproperty.cpp
    src/utils/axisutils.cpp
    src/utils/statsutils.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


# Add shaders
set(SHADER_FILES
)
ivw_group("Shader Files" ${SHADER_FILES})


# Add Unittests
set(TEST_FILES
    tests/unittests/plotting-unittest-main.cpp
    tests/unittests/stats-test.cpp
)
ivw_add_unittest(${TEST_FILES})

# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

# Add shader directory to pack
# ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)
