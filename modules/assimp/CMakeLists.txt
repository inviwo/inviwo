# for now we only import...
SET ( ASSIMP_NO_EXPORT ON CACHE BOOL "Disable Assimp's export functionality." FORCE )

add_subdirectory(ext/assimp)

#--------------------------------------------------------------------
# Inviwo assimp Module
ivw_module(Assimp)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/assimpreader.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/assimpreader.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(inviwo-module-assimp PUBLIC assimp)
target_include_directories(inviwo-module-assimp PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ext/assimp/include>
    #$<INSTALL_INTERFACE:???>
)
ivw_register_license_file(NAME Assimp VERSION 3.0 MODULE Assimp 
    URL http://assimp.sf.net
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/assimp/LICENSE
)
ivw_make_package(InviwoAssimpModule inviwo-module-assimp)