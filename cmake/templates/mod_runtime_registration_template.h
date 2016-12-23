// Automatically generated file do not change!  (see globalmacros.cmake, ivw_private_generate_module_registration_file)
#include <inviwo/core/common/inviwomodulefactoryobject.h>
#include <inviwo/core/util/sharedlibrary.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stringconversion.h>
#if defined(__unix__)
#include <elf.h> // To retrieve rpath
#include <link.h>
#endif

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
#if defined(__APPLE__)
    // Xcode/OSX store library output path in DYLD_LIBRARY_PATH
    if (char* envPaths = std::getenv("DYLD_LIBRARY_PATH")) {
        auto libPaths = splitString(envPaths, ':');
        paths.insert(std::end(paths), std::begin(libPaths), std::end(libPaths));
    }
#elif defined(__unix__)
    // Unix uses LD_LIBRARY_PATH or LD_RUN_PATH
    if (char* envPaths = std::getenv("LD_LIBRARY_PATH")) {
        auto libPaths = splitString(envPaths, ':');
        paths.insert(std::end(paths), std::begin(libPaths), std::end(libPaths));
    }
    if (char* envPaths = std::getenv("LD_RUN_PATH")) {
        auto libPaths = splitString(envPaths, ':');
        paths.insert(std::end(paths), std::begin(libPaths), std::end(libPaths));
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
            offset = static_cast<const char *>(dyn->d_un.d_val);
        }
    }
    if (offset) {
        // Prioritize DT_RUNPATH, DT_RPATH is deprecated
        if (runPath) {
            auto rPaths = splitString(offset + runPath->d_un.d_val, ':');
            paths.insert(std::end(paths), std::begin(rPaths), std::end(rPaths));
        } else if (rPath) {
            auto rPaths = splitString(offset + rPath->d_un.d_val, ':');
            paths.insert(std::end(paths), std::begin(rPaths), std::end(rPaths));
        }
    }
#endif
    return paths;
}

}  //namespace