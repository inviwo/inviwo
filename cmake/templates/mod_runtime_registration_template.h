// Automatically generated file do not change!  (see globalmacros.cmake, ivw_private_generate_module_registration_file)
#include <inviwo/core/common/inviwomodulefactoryobject.h>
#include <inviwo/core/util/sharedlibrary.h>
#include <inviwo/core/util/filesystem.h>

namespace inviwo {

/**
 * \brief Returns paths to search for module libraries
 *
 * @return std::vector<std::string> executable directory and application Modules directory
 * (AppData/Inviwo on windows)
 */
std::vector<std::string> registerAllModules() {
    return std::vector<std::string>{
        inviwo::filesystem::getFileDirectory(inviwo::filesystem::getExecutablePath()),
        inviwo::filesystem::getPath(inviwo::PathType::Modules)};
}

}  //namespace