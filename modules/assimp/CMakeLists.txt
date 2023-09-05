# Inviwo assimp Module
ivw_module(Assimp)

set(HEADER_FILES
    include/modules/assimp/assimpmodule.h
    include/modules/assimp/assimpmoduledefine.h
    include/modules/assimp/assimpreader.h
)
ivw_group("Header Files" ${HEADER_FILES})

set(SOURCE_FILES
    src/assimpmodule.cpp
    src/assimpreader.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})

option(IVW_USE_EXTERNAL_ASSIMP "Assimp is provided externaly" OFF)
if(NOT IVW_USE_EXTERNAL_ASSIMP)
	add_subdirectory(ext/assimp)
	target_include_directories(inviwo-module-assimp PUBLIC 
    	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ext/assimp/include>
	)
    ivw_register_license_file(NAME Assimp VERSION ${Assimp_VERSION} MODULE Assimp 
        URL http://assimp.sf.net
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/assimp/assimp/LICENSE
    )
    ivw_default_install_targets(assimp)
    # assimp adds IrrXML as internal dependency, 
    # which will not be installed unless we add it to the Application component
    ivw_default_install_targets(IrrXML)
else()
    find_package(assimp CONFIG REQUIRED)
    ivw_vcpkg_install(assimp MODULE Assimp)
endif()

target_link_libraries(inviwo-module-assimp PUBLIC assimp::assimp)
