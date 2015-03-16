if(IVW_MODULE_PVM)
    add_subdirectory(ext/tidds)
endif(IVW_MODULE_PVM)

#--------------------------------------------------------------------
# Inviwo PVM Module
ivw_module(PVM)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/mpvmvolumereader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pvmvolumereader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pvmvolumewriter.h
)
ivw_group("Header Files" ${HEADER_FILES})


#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/mpvmvolumereader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pvmvolumereader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pvmvolumewriter.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${MOC_FILES} ${HEADER_FILES})