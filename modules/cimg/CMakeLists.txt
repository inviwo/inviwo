# CImg Module
ivw_module(CImg)

set(HEADER_FILES
    include/modules/cimg/cimglayerreader.h
    include/modules/cimg/cimglayerwriter.h
    include/modules/cimg/cimgmodule.h
    include/modules/cimg/cimgmoduledefine.h
    include/modules/cimg/cimgsavebuffer.h
    include/modules/cimg/cimgutils.h
    include/modules/cimg/cimgvolumereader.h
    include/modules/cimg/processors/layerresampling.h
    include/modules/cimg/tifflayerreader.h
    include/modules/cimg/tiffstackvolumereader.h
)

set(SOURCE_FILES
    src/cimglayerreader.cpp
    src/cimglayerwriter.cpp
    src/cimgmodule.cpp
    src/cimgsavebuffer.cpp
    src/cimgutils.cpp
    src/cimgvolumereader.cpp
    src/processors/layerresampling.cpp
    src/tifflayerreader.cpp
    src/tiffstackvolumereader.cpp
)

# Unit tests
set(TEST_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/cimg-unittest-main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/savetobuffer-test.cpp
)
ivw_add_unittest(${TEST_FILES})

# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES})

find_package(ZLIB REQUIRED)
find_package(JPEG REQUIRED)
find_package(TIFF REQUIRED)
find_package(OpenEXR CONFIG REQUIRED)
find_package(CImg CONFIG REQUIRED)

ivw_vcpkg_install(cimg MODULE CImg)
ivw_vcpkg_install(openexr MODULE CImg)

target_link_libraries(inviwo-module-cimg
    PRIVATE
        CImg::CImg
        ZLIB::ZLIB
        JPEG::JPEG
        TIFF::TIFF
        OpenEXR::OpenEXR
)

target_compile_definitions(inviwo-module-cimg PRIVATE
    cimg_verbosity=0
    cimg_display=0
    cimg_use_cpp11
    cimg_use_jpeg
    cimg_use_zlib
    cimg_use_tiff
    cimg_use_openexr
)




