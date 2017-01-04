// Automatically generated file do not change!  (see globalmacros.cmake, ivw_private_generate_module_registration_file)
#include <inviwo/core/common/inviwomodulefactoryobject.h>
#include <inviwo/core/util/sharedlibrary.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/logcentral.h>

#if defined(__unix__)
#include <elf.h> // To retrieve rpath
#include <link.h>
#endif

namespace inviwo {

/**
 * \brief Returns paths to search for module libraries.
 * All platforms: executable directory and application modules directory
 * (AppData/Inviwo/modules on windows).
 * Platform dependent search directories:
 * OSX: DYLD_LIBRARY_PATH
 * UNIX: LD_LIBRARY_PATH/LD_RUN_PATH, RPATH and "executable directory
 * /../../lib"
 * @return List of paths to directories
 */
std::vector<std::string> registerAllModules() {
    auto paths = std::vector<std::string>{
        inviwo::filesystem::getFileDirectory(inviwo::filesystem::getExecutablePath()),
        inviwo::filesystem::getPath(inviwo::PathType::Modules)};

    // http://unix.stackexchange.com/questions/22926/where-do-executables-look-for-shared-objects-at-runtime
#if defined(__APPLE__)
    // Xcode/OSX store library output path in DYLD_LIBRARY_PATH
    if (char *envPaths = std::getenv("DYLD_LIBRARY_PATH")) {
        auto libPaths = splitString(envPaths, ':');
        paths.insert(std::end(paths), std::begin(libPaths), std::end(libPaths));
    }
#elif defined(__unix__)
    paths.push_back(inviwo::filesystem::getFileDirectory(
        inviwo::filesystem::getExecutablePath()) +
        "/../lib");
    LogInfoCustom("registerAllModules()", "lib: " + paths.back());
        // Unix uses LD_LIBRARY_PATH or LD_RUN_PATH
    if (char *envPaths = std::getenv("LD_LIBRARY_PATH")) {
        auto libPaths = splitString(envPaths, ':');
        paths.insert(std::end(paths), std::begin(libPaths), std::end(libPaths));
        LogInfoCustom("registerAllModules()", "LD_LIBRARY_PATH: " + std::string(envPaths));
    }
    if (char *envPaths = std::getenv("LD_RUN_PATH")) {
        auto libPaths = splitString(envPaths, ':');
        paths.insert(std::end(paths), std::begin(libPaths), std::end(libPaths));
        LogInfoCustom("registerAllModules()", "LD_RUN_PATH: " + std::string(envPaths));
    }
    // Additional paths can be specified in
    // ELF header: RUN_PATH or RPATH
    const ElfW(Dyn) *rPath = nullptr;
    const ElfW(Dyn) *runPath = nullptr;
    const char *offset = nullptr;
    for (const ElfW(Dyn) *dyn = _DYNAMIC; dyn->d_tag != DT_NULL; ++dyn) {
        if (dyn->d_tag == DT_RUNPATH) {
            runPath = dyn;
        } else if (dyn->d_tag == DT_RPATH) {
            rPath = dyn;
        } else if (dyn->d_tag == DT_STRTAB) {
            offset = (const char *)dyn->d_un.d_val;
        }
    }
    if (offset) {
        LogInfoCustom("registerAllModules()", "RPATH: " + std::string(offset + runPath->d_un.d_val));
        // Prioritize DT_RUNPATH, DT_RPATH is deprecated
        if (runPath) {
            auto rPaths = splitString(offset + runPath->d_un.d_val, ':');
            auto execPath = inviwo::filesystem::getExecutablePath();
            for (auto &path : rPaths) {
                replaceInString(path, "$ORIGIN", execPath);
            }
            paths.insert(std::end(paths), std::begin(rPaths), std::end(rPaths));
        } else if (rPath) {
            auto rPaths = splitString(offset + rPath->d_un.d_val, ':');
            auto execPath = inviwo::filesystem::getExecutablePath();
            for (auto &path : rPaths) {
                replaceInString(path, "$ORIGIN", execPath);
            }
            paths.insert(std::end(paths), std::begin(rPaths), std::end(rPaths));
        }
    }
#endif
    return paths;
}

}  //namespace