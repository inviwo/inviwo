# Inviwo Nifti Module
ivw_module(Nifti)

set(HEADER_FILES
    include/modules/nifti/niftimodule.h
    include/modules/nifti/niftimoduledefine.h
    include/modules/nifti/niftireader.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/niftimodule.cpp
    src/niftireader.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Add Unittests
set(TEST_FILES
    tests/unittests/nifti-unittest-main.cpp
    tests/unittests/nifti-test.cpp
)
ivw_add_unittest(${TEST_FILES})

# Create module
ivw_create_module(
    PACKAGES NIFTI
    FILES ${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES}
)

find_package(NIFTI CONFIG REQUIRED)
target_link_libraries(inviwo-module-nifti PRIVATE NIFTI::znz NIFTI::niftiio)
ivw_vcpkg_install(nifticlib MODULE Nifti)
