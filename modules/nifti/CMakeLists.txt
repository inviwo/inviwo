add_subdirectory(ext/nifticlib-2.0.0)

#--------------------------------------------------------------------
# Inviwo Nifti Module
ivw_module(Nifti)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/modules/nifti/niftimodule.h
    include/modules/nifti/niftimoduledefine.h
    include/modules/nifti/niftireader.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/niftimodule.cpp
    src/niftireader.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

target_link_libraries(inviwo-module-nifti PRIVATE inviwo::niftiio inviwo::znz)

# niftio and znz are under same niftilib license 
ivw_register_license_file(ID niftilib NAME "Niftilib" TARGET inviwo::niftiio MODULE Nifti
    URL http://niftilib.sourceforge.net/
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/nifticlib-2.0.0/LICENSE
)

ivw_make_package(InviwoNiftiModule inviwo-module-nifti)
