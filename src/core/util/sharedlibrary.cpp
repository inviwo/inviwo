/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#include <inviwo/core/util/sharedlibrary.h>
#include <inviwo/core/util/filesystem.h>

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
    // Problem: LOAD_WITH_ALTERED_SEARCH_PATH first looks for dependencies in the 
    // executable directory
    //SetDllDirectoryA(nullptr);
    //handle_ = LoadLibraryExA(dstPath.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
    handle_ = LoadLibraryExA(filePath.c_str(), nullptr, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_SEARCH_USER_DIRS);
    
    //handle_ = LoadLibraryExA(filesystem::getFileNameWithExtension(dstPath).c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
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
    handle_ = dlopen(filePath.c_str(), RTLD_NOW | RTLD_GLOBAL);
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
    return dlsym(handle_, name.c_str());
#endif
}


} // namespace

