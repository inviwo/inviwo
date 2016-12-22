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
    
    // http://unix.stackexchange.com/questions/22926/where-do-executables-look-for-shared-objects-at-runtime
    char* dyldPaths = nullptr;
#if defined(__APPLE__)
    // Xcode/OSX store library output path in DYLD_LIBRARY_PATH
    dyldPaths = std::getenv("DYLD_LIBRARY_PATH");
#elif defined(__unix__)
    // Unix uses LD_LIBRARY_PATH instead
    dyldPaths = std::getenv("LD_LIBRARY_PATH");
#endif
    if (dyldPaths) {
        auto dyPaths = splitString(dyldPaths, ':');
        paths.insert(std::end(paths), std::begin(dyPaths), std::end(dyPaths));
    }
    return paths;
}

}  //namespace