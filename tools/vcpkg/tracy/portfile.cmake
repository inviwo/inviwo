include(vcpkg_common_functions)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO wolfpld/tracy
    REF v0.7.3
    SHA512 bfe6f6e01c811b530d36394bbc771fd73a6fbe33087995d26875cdfac497f8b2cd32fce7aafe6c48d891e8891d999d39959d094071bf52ceb0c96ab376c7941a
    HEAD_REF master
)

file(COPY ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt DESTINATION ${SOURCE_PATH})

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS_DEBUG
        -DTRACY_SKIP_INSTALL_HEADERS=ON
        -DTRACY_SKIP_TOOLS=ON
)

vcpkg_install_cmake()
vcpkg_copy_pdbs()
vcpkg_fixup_cmake_targets(CONFIG_PATH share/unofficial-tracy TARGET_PATH share/unofficial-tracy)
vcpkg_copy_tools(TOOL_NAMES TracyProfiler update capture import-chrome AUTO_CLEAN)

# Handle copyright
file(INSTALL ${SOURCE_PATH}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
