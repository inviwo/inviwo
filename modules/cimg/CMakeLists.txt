# Add OpenEXR
add_subdirectory(ext/openexr)
add_subdirectory(ext/cimg)
#--------------------------------------------------------------------
# CImg Module
ivw_module(CImg)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    include/modules/cimg/cimglayerreader.h
    include/modules/cimg/cimglayerwriter.h
    include/modules/cimg/cimgmodule.h
    include/modules/cimg/cimgmoduledefine.h
    include/modules/cimg/cimgsavebuffer.h
    include/modules/cimg/cimgutils.h
    include/modules/cimg/cimgvolumereader.h
)

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    src/cimglayerreader.cpp
    src/cimglayerwriter.cpp
    src/cimgmodule.cpp
    src/cimgsavebuffer.cpp
    src/cimgutils.cpp
    src/cimgvolumereader.cpp
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
    inviwo::tiff
    openexr-half
    openexr-iex
    openexr-iexmath
    openexr-imath
    openexr-ilmthread
    openexr-ilmimf
    openexr-ilmimfutil
    inviwo::cimg
)

target_compile_definitions(inviwo-module-cimg PUBLIC
    cimg_use_cpp11
    cimg_use_jpeg
    cimg_use_zlib
    cimg_use_openexr
    cimg_use_tiff
)

ivw_register_license_file(NAME CImg VERSION 2.1.5 MODULE CImg TYPE CeCILL-C
    URL http://cimg.eu
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/cimg/Licence_CeCILL-C_V1-en.txt
)

ivw_register_license_file(NAME "OpenEXR" TARGET openexr-ilmimf MODULE CImg
    URL "http://www.openexr.com"
    TYPE "Modified BSD License"
    FILES ext/openexr/openexr-2.2.0/LICENSE
)

ivw_register_license_file(NAME libjpeg-turbo TARGET inviwo::libjpeg MODULE CImg
    TYPE "BSD-style, acknowledgement: This software is based in part on the work of the Independent JPEG Group"
    URL www.ijg.org
    FILES ${IVW_EXTENSIONS_DIR}/libjpeg/README
)

ivw_register_license_file(NAME tiff TARGET inviwo::tiff MODULE CImg
    TYPE "The MIT License"
    URL http://www.libtiff.org/
    FILES ${IVW_EXTENSIONS_DIR}/tiff/COPYRIGHT
)

ivw_make_package(InviwoCImgModule inviwo-module-cimg)
