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

option(IVW_USE_EXTERNAL_CIMG  "Cimg is provided externaly" OFF)
if(NOT IVW_USE_EXTERNAL_CIMG)
    add_subdirectory(ext/cimg)
    ivw_register_license_file(NAME CImg VERSION 3.1.1 MODULE CImg TYPE CeCILL-C
        URL http://cimg.eu
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/ext/cimg/Licence_CeCILL-C_V1-en.txt
    )
    target_link_libraries(inviwo-module-cimg PRIVATE inviwo::cimg)
else()
    find_package(CImg CONFIG REQUIRED)
    target_link_libraries(inviwo-module-cimg PRIVATE CImg::CImg)
    ivw_vcpkg_install(cimg MODULE CImg)
endif()

target_link_libraries(inviwo-module-cimg 
    PRIVATE
        ZLIB::ZLIB
        JPEG::JPEG
        TIFF::TIFF
)

target_compile_definitions(inviwo-module-cimg PRIVATE
    cimg_verbosity=0
    cimg_display=0
    cimg_use_cpp11
    cimg_use_jpeg
    cimg_use_zlib
    cimg_use_tiff
)

# The current OpenEXR has some problems with regards to packaging. So for now we make it optional.
# When the packaging is fixed this option should be removed.
option(IVW_ENABLE_OPENEXR  "Add OpenEXR image format support (HDR images)" ON)
option(IVW_USE_EXTERNAL_OPENEXR  "OpenEXR is provided externaly" OFF)
if(IVW_ENABLE_OPENEXR)
    target_compile_definitions(inviwo-module-cimg PRIVATE cimg_use_openexr)
    if(IVW_USE_EXTERNAL_OPENEXR)
        find_package(OpenEXR CONFIG REQUIRED)
        target_link_libraries(inviwo-module-cimg PRIVATE OpenEXR::OpenEXR)
        ivw_vcpkg_install(openexr MODULE CImg)
    else()
        add_subdirectory(ext/openexr)
        ivw_register_license_file(NAME "OpenEXR" TARGET OpenEXR::OpenEXR MODULE CImg
            URL "http://www.openexr.com"
            TYPE "Modified BSD License"
            FILES ext/openexr/openexr/LICENSE.md
        )
        target_link_libraries(inviwo-module-cimg PRIVATE  OpenEXR::OpenEXR)
    endif()
endif()
