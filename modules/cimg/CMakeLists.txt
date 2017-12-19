#--------------------------------------------------------------------
# CImg Module
ivw_module(CImg)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/cimglayerreader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cimglayerwriter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cimgsavebuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cimgutils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cimgvolumereader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ext/cimg/CImg.h
)

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/cimglayerreader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cimglayerwriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cimgsavebuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cimgutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cimgvolumereader.cpp
)

#--------------------------------------------------------------------
# Unit tests
set(TEST_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/cimg-unittest-main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/savetobuffer-test.cpp
)
ivw_add_unittest(${TEST_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(inviwo-module-cimg PUBLIC
    ZLIB::ZLIB
    inviwo::libjpeg
    inviwo::libpng
    inviwo::tiff
    openexr-half
    openexr-iex
    openexr-iexmath
    openexr-imath
    openexr-ilmthread
    openexr-ilmimf
    openexr-ilmimfutil
)

target_compile_definitions(inviwo-module-cimg PUBLIC 
    cimg_use_cpp11
    cimg_use_png
    cimg_use_jpeg
    cimg_use_zlib
    cimg_use_openexr
    cimg_use_tiff
)

ivw_register_license_file(NAME CImg VERSION 2.1.5 MODULE CImg TYPE CeCILL-C
    URL http://cimg.eu
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/cimg/Licence_CeCILL-C_V1-en.txt
)
ivw_make_package(InviwoCImgModule inviwo-module-cimg)

