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

ivw_include_directories(ext/assimp/include)

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})