// Automatically generated file do not change!
#include <inviwo/core/common/inviwomodulefactoryobject.h>

namespace inviwo {
typedef InviwoModuleFactoryObject* (__stdcall *f_getModule)();
std::vector<std::unique_ptr<InviwoModuleFactoryObject>> registerAllModules() {
    std::vector<std::unique_ptr<InviwoModuleFactoryObject>> modules;
    
    auto files = filesystem::getDirectoryContents(filesystem::getFileDirectory(filesystem::getExecutablePath()), filesystem::ListMode::Files);
#if WIN32
    std::string libraryType = "dll";
    // Prevent error mode dialogs from displaying.
    SetErrorMode(SEM_FAILCRITICALERRORS);
#else
    std::string libraryType = "so";
#endif

    for (const auto& filePath : files) {
        if (filesystem::getFileExtension(filePath) == libraryType) {
            try {
                std::unique_ptr<SharedLibrary> sharedLib = std::unique_ptr<SharedLibrary>(new SharedLibrary(filePath));
                f_getModule moduleFunc = (f_getModule)sharedLib->findSymbol("createModule");
                modules.emplace_back(moduleFunc());
                //modules.back()->library_ = std::move(sharedLib);
            } catch (Exception ex) {
                LogError(ex.message());
            }
        }
    }

    return modules;
}

}  //namespace