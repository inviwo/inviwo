vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO LLNL/units
    REF 153aefde0df7fbb58dd8b6acd312c2ebd93cc8a6  
    SHA512 cee00b847750ecc2374288cfc4817aa80059336c875e10af0a43373cf54dff32868d273854de74150eb7c80c555c0af9d3655ffeb801392a8c0ac550b3e1d47d
    HEAD_REF main
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"

    OPTIONS
        -DUNITS_CMAKE_PROJECT_NAME=LLNL-UNITS
        -DUNITS_ENABLE_TESTS=OFF
        -DUNITS_BUILD_FUZZ_TARGETS=OFF
        -DUNITS_ENABLE_ERROR_ON_WARNINGS=OFF
        -DUNITS_ENABLE_EXTRA_COMPILER_WARNINGS=OFF
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME llnl-units CONFIG_PATH lib/cmake/llnl-units)
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
