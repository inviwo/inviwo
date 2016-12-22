// Automatically generated file do not change!  (see globalmacros.cmake, ivw_private_generate_module_registration_file)
#include <inviwo/core/common/inviwomodulefactoryobject.h>
#include <inviwo/core/util/sharedlibrary.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stringconversion.h>

namespace inviwo {

/**
 * \brief Returns paths to search for module libraries
 *
 * @return std::vector<std::string> executable directory and application Modules directory
 * (AppData/Inviwo on windows)
 */
std::vector<std::string> registerAllModules() {
    auto paths = std::vector<std::string>{
        inviwo::filesystem::getFileDirectory(inviwo::filesystem::getExecutablePath()),
        inviwo::filesystem::getPath(inviwo::PathType::Modules)};
    // Xcode stores library output path in DYLD_LIBRARY_PATH
    auto dyldPaths = std::getenv("DYLD_LIBRARY_PATH");
    if (dyldPaths) {
        auto dyPaths = splitString(dyldPaths, ':');
        paths.insert(std::end(paths), std::begin(dyPaths), std::end(dyPaths));
    }
    return paths;
}

}  //namespace