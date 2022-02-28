vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO pybind/pybind11
    REF 4c4b33f14215f8992e2c67ace2a7a4c114c48bb9 # v2.9.1
    SHA512 fbdeb00e677266a033bda9d0135fbbbff38205a6172ae03659d88e1815646d8e628fb5c077d34d24bdb821200eb16d06c86036f2baeddbbce113128a353ceae0
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
