// Automatically generated file do not change!
#include <inviwo/core/common/inviwomodulefactoryobject.h>
@MODULE_HEADERS@

namespace inviwo {

std::vector<std::unique_ptr<InviwoModuleFactoryObject>> registerAllModules() {
    std::vector<std::unique_ptr<InviwoModuleFactoryObject>> modules;

    std::string subfolder;
#if WIN32
# ifdef _DEBUG
    subfolder = "/../../bin/Debug";
# else
    subfolder = "/../../bin/Release";
# endif
#endif
    auto files = filesystem::getDirectoryContents(filesystem::getWorkingDirectory() + subfolder, filesystem::ListMode::Files);
#if WIN32
    std::string libraryType = "dll";
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
                f_getModule moduleFunc = (f_getModule)GetProcAddress(hGetProcIDDLL, "getModule");
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