#pragma once
// Automatically generated file do not change!
#include <string_view>
#include <inviwo/core/common/version.h>

namespace inviwo::build {
    inline constexpr std::string_view sourceDirectory{"@IVW_ROOT_DIR@"};
    inline constexpr std::string_view binaryDirectory{"@IVW_BINARY_DIR@"};
    inline constexpr Version version{std::string_view{"@IVW_VERSION@"}};
    inline constexpr std::string_view generator{"@CMAKE_GENERATOR@"};
    inline constexpr std::string_view compiler{"@CMAKE_CXX_COMPILER_ID@"};
    inline constexpr std::string_view compilerVersion{"@CMAKE_CXX_COMPILER_VERSION@"};

namespace vcpkg {
    inline constexpr std::string_view triplet{"@VCPKG_TARGET_TRIPLET@"};
    inline constexpr std::string_view installDir{"@VCPKG_INSTALLED_DIR@"};
    inline constexpr std::string_view features{"@VCPKG_MANIFEST_FEATURES@"};
}  // namespace vcpkg

namespace python {
    inline constexpr std::string_view sitelib{"@Python3_SITELIB@"};
}  // namespace python

#ifdef CMAKE_BUILD_TYPE
    inline constexpr std::string_view configuration{CMAKE_BUILD_TYPE};
#else
    inline constexpr std::string_view configuration{"Unknown"};
#endif

}  // namespace inviwo::build
