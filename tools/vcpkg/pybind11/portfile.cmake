vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO pybind/pybind11
    REF ffa346860b306c9bbfb341aed9c14c067751feb8 # v2.9.1
    SHA512 c2d52ff5f161adaf4b8a658960688fff5c652fbb45b99a90f39d2214751bdca22cd440de0f80f7cee2b26bb0e9c59af83d530bbda9e721a34eb58a2a947307c3
    HEAD_REF master
)   

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
        -DBUILD_TESTING=OFF
        -DPYBIND11_TEST=OFF
        -DPYBIND11_FINDPYTHON=ON
        -DPYTHON_IS_DEBUG=OFF
)

vcpkg_install_cmake()
vcpkg_fixup_cmake_targets(CONFIG_PATH share/cmake/pybind11)

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/)

# copy license
file(INSTALL ${SOURCE_PATH}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
