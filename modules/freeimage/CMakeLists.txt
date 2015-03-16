if(IVW_MODULE_FREEIMAGE)
    add_subdirectory(ext/freeimage)
endif(IVW_MODULE_FREEIMAGE)

#--------------------------------------------------------------------
# FreeImage Module
ivw_module(FreeImage)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/freeimageutils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/freeimagereader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/freeimagewriter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/tonemappingcpuprocessor.h
)
ivw_group("Header Files" ${HEADER_FILES})


#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/freeimageutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/freeimagereader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/freeimagewriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/tonemappingcpuprocessor.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})