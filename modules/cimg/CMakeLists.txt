# Add OpenEXR
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
    include/modules/cimg/tifflayerreader.h
    include/modules/cimg/tiffstackvolumereader.h
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
    src/tifflayerreader.cpp
    src/tiffstackvolumereader.cpp
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

find_package(ZLIB REQUIRED)
find_package(JPEG REQUIRED)
find_package(TIFF REQUIRED)

target_link_libraries(inviwo-module-cimg 
    PRIVATE
        inviwo::cimg   
        ZLIB::ZLIB
        JPEG::JPEG
        TIFF::TIFF
)

target_compile_definitions(inviwo-module-cimg PRIVATE
    cimg_use_cpp11
    cimg_use_jpeg
    cimg_use_zlib
    cimg_use_tiff
)

ivw_register_license_file(NAME CImg VERSION 2.1.5 MODULE CImg TYPE CeCILL-C
    URL http://cimg.eu
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/cimg/Licence_CeCILL-C_V1-en.txt
)

ivw_register_license_file(NAME libjpeg-turbo TARGET JPEG::JPEG MODULE CImg
    TYPE "BSD-style, acknowledgement: This software is based in part on the work of the Independent JPEG Group"
    URL www.ijg.org
    FILES ${IVW_EXTENSIONS_DIR}/libjpeg/README
)

ivw_register_license_file(NAME tiff TARGET TIFF::TIFF MODULE CImg
    TYPE "The MIT License"
    URL http://www.libtiff.org/
    FILES ${IVW_EXTENSIONS_DIR}/tiff/COPYRIGHT
)


# The current OpenEXR has some problems with regards to packaging. So for now we make it optional.
# When the packaging is fixed this option should be removed.
option(IVW_CIMG_USE_OPENEXR  "Add OpenEXR image format support (HDR images) " ON)
if(IVW_CIMG_USE_OPENEXR)
    add_subdirectory(ext/openexr)
    
    ivw_register_license_file(NAME "OpenEXR" TARGET OpenEXR::IlmImf MODULE CImg
        URL "http://www.openexr.com"
        TYPE "Modified BSD License"
        FILES ext/openexr/openexr/LICENSE
    )
    target_link_libraries(inviwo-module-cimg 
        PRIVATE
            IlmBase::Half
            IlmBase::Iex
            IlmBase::IexMath
            IlmBase::Imath
            IlmBase::IlmThread
            OpenEXR::IlmImf
            OpenEXR::IlmImfUtil
    )
    target_include_directories(inviwo-module-cimg PRIVATE 
        ${CMAKE_CURRENT_BINARY_DIR}/ext/openexr/openexr/IlmBase/config
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/openexr/IlmBase/Half
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/openexr/IlmBase/Iex
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/openexr/IlmBase/IexMath
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/openexr/IlmBase/Imath
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/openexr/IlmBase/IlmThread
        ${CMAKE_CURRENT_BINARY_DIR}/ext/openexr/openexr/OpenEXR/config
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/openexr/OpenEXR/IlmImf
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/openexr/openexr/OpenEXR/IlmImfUtil
    )
    target_compile_definitions(inviwo-module-cimg PRIVATE cimg_use_openexr)
endif()

ivw_make_package(InviwoCImgModule inviwo-module-cimg)
