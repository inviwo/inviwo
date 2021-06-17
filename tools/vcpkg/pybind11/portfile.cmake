vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO pybind/pybind11
    REF 4f6191264697872a3711eff99fa8c1c8e87c00b3 # v2.6.0
    SHA512 de327ce7da3c30fc53935464a927deb90893e085a729c9e0ea653f3a7c66d3f885927b37773d0bcb6baad116e9e1e7526cad1edfc3d8f9b7ca4f80643d0e3e56
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
