#--------------------------------------------------------------------
# Inviwo discrete data Module
ivw_module(DiscreteData)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/analyticchannel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/bufferchannel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/channeliterator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/channeliterator.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/connectioniterator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/connectivity.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datachannel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/datachannel.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/elementiterator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/structuredgrid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/util.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/connectioniterator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/connectivity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/datachannel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/elementiterator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/structuredgrid.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})
