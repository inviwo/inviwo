#--------------------------------------------------------------------
# CImg Module
ivw_module(CImg)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/cimg/CImg.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cimgutils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cimglayerreader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cimglayerwriter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cimgvolumereader.h
)

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/cimgutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cimglayerreader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cimglayerwriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cimgvolumereader.cpp
)

#--------------------------------------------------------------------
# Define defintions
ivw_add_definition(cimg_use_cpp11)
ivw_add_definition(cimg_use_png)
ivw_add_definition(cimg_use_jpeg)
ivw_add_definition(cimg_use_zlib)
ivw_add_definition(cimg_use_openexr)

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})
