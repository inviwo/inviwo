#--------------------------------------------------------------------
# Inviwo discrete data Module
ivw_module(DiscreteData)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/channels/analyticchannel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/channels/bufferchannel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/channels/channelgetter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/channels/channelgetter.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/channels/channeliterator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/channels/channeliterator.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/channels/datachannel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/channels/datachannel.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/connectivity/cell.h
    ${CMAKE_CURRENT_SOURCE_DIR}/connectivity/connectioniterator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/connectivity/connectivity.h
    ${CMAKE_CURRENT_SOURCE_DIR}/connectivity/elementiterator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/connectivity/euclideanmeasure.h
    ${CMAKE_CURRENT_SOURCE_DIR}/connectivity/periodicgrid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/connectivity/structuredgrid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dataset.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dataset.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/util.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/channels/datachannel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/connectivity/connectioniterator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/connectivity/connectivity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/connectivity/elementiterator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/connectivity/periodicgrid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/connectivity/structuredgrid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dataset.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/data-unittest-main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/data-test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/dataset-test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/data-access-test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/example-code.cpp
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})
