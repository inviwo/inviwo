# Header-only library
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO boost-ext/sml
    REF "v${VERSION}"
    SHA512 ac40d4c273ea91d52419e88c27c079efbcb5d29d59690b82840b69091fdd16dc72d90aa661c1bd340c448904dc59837ca1d284d0f144f254fcaf11f4a6998649
    HEAD_REF master
)

vcpkg_cmake_configure(
    SOURCE_PATH ${SOURCE_PATH}
    OPTIONS
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(PACKAGE_NAME sml CONFIG_PATH lib/cmake/sml)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/lib")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.md")
