vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO mity/md4c
    REF release-0.5.2
    SHA512 30607ba39d6c59329f5a56a90cd816ff60b82ea752ac2b9df356d756529cfc49170019fae5df32fa94afc0e2a186c66eaf56fa6373d18436c06ace670675ba85
    HEAD_REF master
)

vcpkg_cmake_configure(
    SOURCE_PATH ${SOURCE_PATH}
    OPTIONS
        -DBUILD_MD2HTML_EXECUTABLE=OFF
)

vcpkg_cmake_install()
vcpkg_copy_pdbs()
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/md4c)

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.md")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/lib/pkgconfig")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/lib/pkgconfig")
