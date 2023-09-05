vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO petersteneteg/warn
    REF 4e5937bc3287bcbd16aa4540043dca740a3a487a  
    SHA512 2ec25774db3d318affd40e2e3b196cecb7170ed50adbf4bd53f6e0647eb9301cfce5f7bb964e996b888bb7b6cca237e6761ce3d4540cdba22cf6ce7856923c68
    HEAD_REF master
)

# vcpkg does not handle submodules so we have to do it manually
vcpkg_from_github(
    OUT_SOURCE_PATH CW_SOURCE_PATH
    REPO pkolbus/compiler-warnings
    REF f141a7b932778d282b29838e0cdf0c983a57253d  
    SHA512 67685b9473c6554e17d808b734b42a849c248d8826fdb5c82eca617ab99dd5d77bc1a6d032066a07c71e11f2f663f79414dcb8ccb7342afa4c54c4ac8027453a
    HEAD_REF master
)
file(COPY ${CW_SOURCE_PATH} DESTINATION ${SOURCE_PATH}/ext/barro)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DWARN_INSTALL=ON
        -DWARN_EXTRA_WARNINGS_FILE=${CURRENT_PORT_DIR}/extra_warnings.md
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME warn CONFIG_PATH share/warn)
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
