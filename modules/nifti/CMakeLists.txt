add_subdirectory(ext/nifticlib-2.0.0)

set(NIFTI_INCLUDE_DIR ${NIFTILIB_SOURCE_DIR} ${ZNZLIB_SOURCE_DIR} PARENT_SCOPE)
set(NIFTI_INCLUDE_DIR ${NIFTILIB_SOURCE_DIR} ${ZNZLIB_SOURCE_DIR})

set(NIFTI_FOUND 1 PARENT_SCOPE)
set(NIFTI_LIBRARIES niftiio znz PARENT_SCOPE)
set(NIFTI_LIBRARIES niftiio znz)
#--------------------------------------------------------------------
# Inviwo Nifti Module
ivw_module(Nifti)
ivw_include_directories(${NIFTI_INCLUDE_DIR})
add_dependency_libs_to_module(${NIFTI_LIBRARIES})
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