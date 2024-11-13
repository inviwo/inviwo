vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO petersteneteg/warn
    REF 12492b5f78005cc978a200bf8b0e57929a19976a
    SHA512 ee85afa2360ee0a83c6f37d6e384f1da25d642d2eb03ffb12128fa80e0c22ff36ade09dfddf5025037da73368f739ac90d72cf14d0a19eb2adec8aae2bb21a18
    HEAD_REF master
)

# vcpkg does not handle submodules so we have to do it manually
vcpkg_from_github(
    OUT_SOURCE_PATH CW_SOURCE_PATH
    REPO pkolbus/compiler-warnings
    REF cfed13d6aecd6877606b5c24b8a683c86e5969b8
    SHA512 cc57481301c1c57a27419c96817a36fa2d9195eaf04f75e5c51ad5260c7a82323ffe22e99cde0df1a6b084be027c1a70bc85d94918adbcfd5944f5b87082fcf2
    HEAD_REF master
)
file(COPY ${CW_SOURCE_PATH}/ DESTINATION ${SOURCE_PATH}/ext/barro)

vcpkg_find_acquire_program(PYTHON3)
get_filename_component(PYTHON3_DIR "${PYTHON3}" DIRECTORY)
vcpkg_add_to_path("${PYTHON3_DIR}")

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DPython3_ROOT_DIR="${PYTHON3_DIR}"
        -DWARN_INSTALL=ON
        -DWARN_EXTRA_WARNINGS_FILE=${CURRENT_PORT_DIR}/extra_warnings.md
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME warn CONFIG_PATH share/warn)
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
