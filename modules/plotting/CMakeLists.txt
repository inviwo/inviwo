#--------------------------------------------------------------------
# Inviwo Plotting Module
ivw_module(Plotting)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/modules/plotting/datastructures/axisdata.h
    include/modules/plotting/datastructures/axissettings.h
    include/modules/plotting/datastructures/column.h
    include/modules/plotting/datastructures/dataframe.h
    include/modules/plotting/datastructures/dataframeutil.h
    include/modules/plotting/datastructures/datapoint.h
    include/modules/plotting/datastructures/majortickdata.h
    include/modules/plotting/datastructures/majorticksettings.h
    include/modules/plotting/datastructures/minortickdata.h
    include/modules/plotting/datastructures/minorticksettings.h
    include/modules/plotting/datastructures/plottextdata.h
    include/modules/plotting/datastructures/plottextsettings.h
    include/modules/plotting/plottingmodule.h
    include/modules/plotting/plottingmoduledefine.h
    include/modules/plotting/processors/csvsource.h
    include/modules/plotting/processors/dataframecolumntocolorvector.h
    include/modules/plotting/processors/dataframeexporter.h
    include/modules/plotting/processors/imagetodataframe.h
    include/modules/plotting/processors/syntheticdataframe.h
    include/modules/plotting/processors/volumesequencetodataframe.h
    include/modules/plotting/processors/volumetodataframe.h
    include/modules/plotting/properties/axisproperty.h
    include/modules/plotting/properties/categoricalaxisproperty.h
    include/modules/plotting/properties/dataframeproperty.h
    include/modules/plotting/properties/marginproperty.h
    include/modules/plotting/properties/optionconverter.h
    include/modules/plotting/properties/plottextproperty.h
    include/modules/plotting/properties/tickproperty.h
    include/modules/plotting/utils/axisutils.h
    include/modules/plotting/utils/csvreader.h
    include/modules/plotting/utils/statsutils.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/datastructures/axisdata.cpp
    src/datastructures/axissettings.cpp
    src/datastructures/column.cpp
    src/datastructures/dataframe.cpp
    src/datastructures/dataframeutil.cpp
    src/datastructures/majortickdata.cpp
    src/datastructures/majorticksettings.cpp
    src/datastructures/minortickdata.cpp
    src/datastructures/minorticksettings.cpp
    src/datastructures/plottextdata.cpp
    src/datastructures/plottextsettings.cpp
    src/plottingmodule.cpp
    src/processors/csvsource.cpp
    src/processors/dataframecolumntocolorvector.cpp
    src/processors/dataframeexporter.cpp
    src/processors/imagetodataframe.cpp
    src/processors/syntheticdataframe.cpp
    src/processors/volumesequencetodataframe.cpp
    src/processors/volumetodataframe.cpp
    src/properties/axisproperty.cpp
    src/properties/categoricalaxisproperty.cpp
    src/properties/dataframeproperty.cpp
    src/properties/marginproperty.cpp
    src/properties/plottextproperty.cpp
    src/properties/tickproperty.cpp
    src/utils/axisutils.cpp
    src/utils/csvreader.cpp
    src/utils/statsutils.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})


#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
    #${CMAKE_CURRENT_SOURCE_DIR}/glsl/plottingprocessor.frag
)
ivw_group("Shader Files" ${SHADER_FILES})


#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/plotting-unittest-main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/stats-test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/csvreader-test.cpp
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

#--------------------------------------------------------------------
# Add shader directory to pack
# ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)
