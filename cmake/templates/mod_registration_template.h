// Automatically generated file do not change!
#include <inviwo/core/common/inviwomodulefactoryobject.h>
#include <windows.h>

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
            HINSTANCE hGetProcIDDLL = LoadLibraryA(filePath.c_str());

            if (!hGetProcIDDLL) {
                //std::cout << "could not load the dynamic library" << std::endl;
                //return EXIT_FAILURE;
                FreeLibrary(hGetProcIDDLL);
            }
            else {
                f_getModule moduleFunc = (f_getModule)GetProcAddress(hGetProcIDDLL, "createModule");
                if (!moduleFunc) {
                    //std::cout << "could not locate the function" << std::endl;
                    FreeLibrary(hGetProcIDDLL);
                }
                else {
                    modules.emplace_back(moduleFunc());
                }
            }
        }
    }

    return modules;
}

}  //namespace