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
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

option(IVW_USE_EXTERNAL_NIFTI "Nifticlib is provided externaly" OFF)
if(NOT IVW_USE_EXTERNAL_NIFTI)
	add_subdirectory(ext/nifticlib-2.0.0)
	target_link_libraries(inviwo-module-nifti PRIVATE inviwo::niftiio inviwo::znz)

	# niftio and znz are under same niftilib license 
	ivw_register_license_file(ID niftilib NAME "Niftilib" TARGET inviwo::niftiio MODULE Nifti
	    URL http://niftilib.sourceforge.net/
    	FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/nifticlib-2.0.0/LICENSE
	)
else()
    find_package(NIFTI CONFIG REQUIRED)
    target_link_libraries(inviwo-module-nifti PRIVATE NIFTI::znz NIFTI::niftiio)
    ivw_vcpkg_install(nifticlib MODULE Nifti) 
endif()

