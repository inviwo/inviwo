/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2016 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <inviwo/core/common/inviwomodulefactoryobject.h>
#include <inviwo/core/util/assertion.h>



namespace inviwo {

SharedLibrary::SharedLibrary(std::string filePath)
    : filePath_(filePath)
{
#if WIN32
    std::string dir = filesystem::getFileDirectory(filePath);
    std::string tmpDir = filesystem::getWorkingDirectory();
    //std::string tmpDir = dir + "/tmp";
    if (!filesystem::directoryExists(tmpDir)) {
        filesystem::createDirectoryRecursively(tmpDir);
    }
   
    std::string dstPath = tmpDir + "/" +  filesystem::getFileNameWithExtension(filePath);
    //{
    //    // Load a copy of the file to make sure that
    //    // we can overwrite the file.
    //    std::ifstream  src(filePath_, std::ios::binary);
    //    std::ofstream  dst(dstPath, std::ios::binary);

    //    dst << src.rdbuf();
    //}
    handle_ = LoadLibraryExA(dstPath.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
    //handle_ = LoadLibraryExA(dstPath.c_str(), nullptr);
    //handle_ = LoadLibraryA(dstPath.c_str());
    //handle_ = LoadLibraryA(filePath.c_str());
    if (!handle_) {
        auto error = GetLastError();
        std::ostringstream errorStream;
        LPVOID errorText;
        
        auto outputSize = FormatMessage(
            // use system message tables to retrieve error text
            FORMAT_MESSAGE_FROM_SYSTEM
            // allocate buffer on local heap for error text
            | FORMAT_MESSAGE_ALLOCATE_BUFFER
            // Important! will fail otherwise, since we're not 
            // (and CANNOT) pass insertion parameters
            | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&errorText,
            0, NULL);
        if (errorText != nullptr) {
            std::string errorString(static_cast<const char *>(errorText), outputSize + 1);
            errorStream << errorString;
            //errorStream << static_cast<const char *>(errorText);
            // release memory allocated by FormatMessage()
            LocalFree(errorText);
        }

        throw Exception("Failed to load library: " + filePath + "\n Error: " + errorStream.str());
    }
#else 
    handle_ = dlopen(name, RTLD_NOW | RTLD_GLOBAL);
    if (!handle_) {
        throw Exception("Failed to load library: " + filePath);
    }
#endif

}
SharedLibrary::~SharedLibrary() {
#if WIN32
    FreeLibrary(handle_);
#else 
    dlclose(handle_);
#endif
}

void* SharedLibrary::findSymbol(std::string name) {
#if WIN32
    return GetProcAddress(handle_, "createModule");
#else 
    return dlsym(handle_, name);
#endif
}


InviwoModuleFactoryObject::InviwoModuleFactoryObject(
    const std::string& name, const std::string& version, const std::string& description, const std::string& inviwoCoreVersion, 
    std::vector<std::string> dependencies, std::vector<std::string> dependenciesVersion)
    : name_(name), version_(version), description_(description), inviwoCoreVersion_(inviwoCoreVersion), depends_(dependencies), dependenciesVersion_(dependenciesVersion) {
    if (depends_.size() != dependenciesVersion_.size()) {
        throw Exception("Each module dependency must have a version");
    }

}

InviwoModuleFactoryObject::~InviwoModuleFactoryObject() {
    
}

void InviwoModuleFactoryObject::setSharedLibrary(std::unique_ptr<SharedLibrary>& library) {
    //library_ = std::move(library);
    //if (library_) {
    //    startFileObservation(library_->getFilePath());
    //}
}
ModuleLibraryObserver::ModuleLibraryObserver() {
    InviwoApplication::getPtr()->registerFileObserver(this);
}
ModuleLibraryObserver::ModuleLibraryObserver(std::string moduleName) : observedModuleName(moduleName) {
    InviwoApplication::getPtr()->registerFileObserver(this);
};

ModuleLibraryObserver::ModuleLibraryObserver(ModuleLibraryObserver&& other): FileObserver(other), observedModuleName(std::move(other.observedModuleName)) {
    InviwoApplication::getPtr()->unRegisterFileObserver(&other);
    InviwoApplication::getPtr()->registerFileObserver(this);
}
ModuleLibraryObserver::~ModuleLibraryObserver() {
    stopFileObservation(observedModuleName);
    InviwoApplication::getPtr()->unRegisterFileObserver(this);
}

typedef InviwoModuleFactoryObject* (__stdcall *f_getModule)();
void ModuleLibraryObserver::fileChanged(const std::string& fileName) {
    // Serialize network
    std::stringbuf buf;
    std::iostream stream(&buf);
    InviwoApplication* app = InviwoApplication::getPtr();
    Serializer xmlSerializer(app->getBasePath());
    
    try {

        InviwoApplication::getPtr()->getProcessorNetwork()->serialize(xmlSerializer);
        xmlSerializer.writeFile(stream);
    }
    catch (SerializationException& exception) {
        util::log(exception.getContext(),
            "Unable to save network due to " + exception.getMessage(),
            LogLevel::Error);
        return;
    }
    app->clearModules();
    auto observedFiles = getFiles();
    for (auto file: observedFiles) {
        stopFileObservation(file);
    }

    app->registerModulesFromDynamicLibraries(std::vector<std::string>(1, filesystem::getExecutablePath()));
    for (auto file : observedFiles) {
        startFileObservation(file);
    }
    //// Unregister dependent modules
    //const auto& moduleFactories = app->getModuleFactoryObjects();
    ////auto toDeregister = app->findDependentModules(toLower(observedModule_->name_));
    //std::vector<std::string> toDeregister;
    //for (const auto& mod : app->getModules()) {
    //    if (dynamic_cast<InviwoCore*>(mod.get()) == nullptr)
    //        toDeregister.push_back(mod->getIdentifier());
    //}
    ////std::vector<InviwoModuleFactoryObject*> dependentModuleFactories;
    ////
    //
    ////for (auto m : toDeregister) {
    ////    auto it = std::find_if(std::begin(moduleFactories), std::end(moduleFactories), [&](const auto& moduleFactory) {
    ////        return toLower(m) == toLower(moduleFactory->name_);
    ////    });
    ////    if (it != std::end(moduleFactories)) {
    ////        dependentModuleFactories.push_back(it->get());
    ////    }
    ////}

    //// Unregister module
    //for (auto m : toDeregister) {
    //    app->unregisterModule(toLower(m));
    //}
    ////app->unregisterModule(toLower(observedModule_->name_));
    //// Unload so/dll
    //auto it = std::find_if(std::begin(moduleFactories), std::end(moduleFactories), [&](const auto& moduleFactory) {
    //            return toLower(observedModuleName) == toLower(moduleFactory->name_);
    //        });
    //if (it != std::end(moduleFactories)) {
    //    *it = nullptr;
    //    std::unique_ptr<SharedLibrary> sharedLib = std::unique_ptr<SharedLibrary>(new SharedLibrary(fileName));
    //    f_getModule moduleFunc = (f_getModule)sharedLib->findSymbol("createModule");
    //    *it = moduleFunc();
    //    //it->setSharedLibrary(sharedLib);
    //}
    ////std::map<InviwoModuleFactoryObject*, std::string> dependentModuleLibPaths;
    ////for (auto mod : dependentModuleFactories) {
    ////    dependentModuleLibPaths[mod] = mod->library_->getFilePath();
    ////    mod->library_ = nullptr;
    ////}
    //
    ////std::string modPath = fileName;
    ////observedModule_->library_ = nullptr;
    ////// Load so/dll
    //////for (auto mod : dependentModuleLibPaths) {
    //////    mod.first->library_ = std::unique_ptr<SharedLibrary>(new SharedLibrary(mod.second));
    //////}
    ////observedModule_->library_ = std::unique_ptr<SharedLibrary>(new SharedLibrary(modPath));
    //// Register modules
    //app->registerModules(moduleFactories);
    // De-serialize network
    try {
        // Lock the network that so no evaluations are triggered during the de-serialization
        NetworkLock lock(InviwoApplication::getPtr()->getProcessorNetwork());
        Deserializer xmlDeserializer(app, stream, app->getBasePath());
        InviwoApplication::getPtr()->getProcessorNetwork()->deserialize(xmlDeserializer);
    }
    catch (SerializationException& exception) {
        util::log(exception.getContext(),
            "Unable to save network due to " + exception.getMessage(),
            LogLevel::Error);
        return;
    }
}

} // namespace

