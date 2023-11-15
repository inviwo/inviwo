if (TARGET_TRIPLET MATCHES "^x86")
    message(WARNING "The CRoaring authors recommend users of this lib against using a 32-bit build.")
endif ()

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO RoaringBitmap/CRoaring
    REF 8fc18acd32481810acc7166e3faa98be5e1b5675

    SHA512 d1f3a508ece69e5b1ef94f69e34dbb6922d2077d3d3d38deedddc7d6f3a0c2ad152eb8a96b0cd140f41d4df9f2a43b58f8d657cf34995984bf87a07293ae8fc5
    HEAD_REF master
)

string(COMPARE EQUAL "${VCPKG_LIBRARY_LINKAGE}" "static" ROARING_BUILD_STATIC)

vcpkg_cmake_configure(
    SOURCE_PATH ${SOURCE_PATH}
    DISABLE_PARALLEL_CONFIGURE
    OPTIONS
        -DROARING_BUILD_STATIC=${ROARING_BUILD_STATIC}
        -DENABLE_ROARING_TESTS=OFF
)

vcpkg_cmake_install()

vcpkg_copy_pdbs()

vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/${PORT})

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)

vcpkg_fixup_pkgconfig()

# Handle copyright
configure_file(${SOURCE_PATH}/LICENSE ${CURRENT_PACKAGES_DIR}/share/${PORT}/copyright COPYONLY)
