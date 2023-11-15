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

#ifdef CMAKE_BUILD_TYPE
    inline constexpr std::string_view configuration{CMAKE_BUILD_TYPE};
#else
    inline constexpr std::string_view configuration{"Unknown"};
#endif

}
