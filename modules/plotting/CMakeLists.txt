#--------------------------------------------------------------------
# Inviwo Plotting Module
ivw_module(Plotting)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/column.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/dataframe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/datapoint.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/csvsource.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/dataframecolumntocolorvector.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/syntheticdataframe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/axisproperty.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/dataframeproperty.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/marginproperty.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/plottextproperty.h
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/tickproperty.h
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/axisutils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/csvreader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/statsutils.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/column.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/dataframe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/csvsource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/dataframecolumntocolorvector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/syntheticdataframe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/axisproperty.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/dataframeproperty.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/marginproperty.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/plottextproperty.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/properties/tickproperty.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/axisutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/csvreader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/statsutils.cpp
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
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

#--------------------------------------------------------------------
# Add shader directory to pack
# ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)