vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO pybind/pybind11
    REF aed215c4d49532a1b162ea11cd96f77bd3540fed # v2.13.6 2025-02-24 + smart_holder
    SHA512 15d4619d97226fc3de2125cfcb845c5498c1288e149e26bc0eb6588bb2f96ad4270f3ced1fb6f591934ba487e5e2dc6bfdbab190ab7a067aee60e7129ad73890
    HEAD_REF smart_holder
    PATCHES
        include-fix.patch
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DPYBIND11_TEST=OFF
        # Disable all Python searching, Python required only for tests
        -DPYBIND11_NOPYTHON=ON
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(CONFIG_PATH "share/cmake/pybind11")
vcpkg_fixup_pkgconfig()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/")

file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
