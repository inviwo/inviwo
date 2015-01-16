#--------------------------------------------------------------------
# Inviwo Example Module
ivw_module(Example)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/exampleprocessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/exampleprogressbar.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/exampleprocessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/exampleprogressbar.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})