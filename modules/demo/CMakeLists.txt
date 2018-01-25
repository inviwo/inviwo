#--------------------------------------------------------------------
# Inviwo Demo Module
ivw_module(Demo)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/democontroller.h
    ${CMAKE_CURRENT_SOURCE_DIR}/democontrollerobserver.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/democontroller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/democontrollerobserver.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})
