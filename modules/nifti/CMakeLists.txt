add_subdirectory(ext/nifticlib-2.0.0)

#--------------------------------------------------------------------
# Inviwo Nifti Module
ivw_module(Nifti)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/niftireader.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/niftireader.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

target_link_libraries(inviwo-module-nifti PRIVATE inviwo::niftiio inviwo::znz)

ivw_make_package(InviwoNiftiModule inviwo-module-nifti)