/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2022 Inviwo Foundation
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
#include <inviwo/core/util/stringconversion.h>  // splitString
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/logcentral.h>
#include <codecvt>
#include <locale>
#include <algorithm>

#if WIN32
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <psapi.h>
#else
#include <dlfcn.h>
#endif

#if defined(__unix__)
#include <elf.h>  // To retrieve rpath
#include <link.h>
#endif

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

namespace inviwo {

SharedLibrary::SharedLibrary(const std::string& filePath) : filePath_(filePath) {
#if WIN32

    // Load library and search for dependencies in
    // We load libraries using 'LOAD_WITH_ALTERED_SEARCH_PATH'
    // https://learn.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-search-order#alternate-search-order-for-desktop-applications
    // This requires us to pass in absolute paths. And will use the systems default search paths
    // for finding dependent libraries. Except that the directory of the loaded dll will be added
    // before all other search paths. Hence the search order goes like this
    //  * The directory of the loaded DLL
    //  * The current working directory
    //  * The systems dirs, windows/system32, windows/
    //  * The user environment PATH variable
    //
    // Note 1: Since the directory of the loaded DLL is searched first, this enables us to use a
    // temporary directory when loading dlls, which thereby allows the original ones to be
    // overwritten while the application is running.

    // LoadLibrary requires '\'
    replaceInString(filePath_, "/", "\\");

    handle_ =
        LoadLibraryExW(util::toWstring(filePath_).c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);

    if (!handle_) {
        auto error = GetLastError();
        std::wstring errorStr;
        LPVOID errorText;

        auto outputSize = FormatMessageW(
            FORMAT_MESSAGE_FROM_SYSTEM  // use system message tables to retrieve error text
                | FORMAT_MESSAGE_ALLOCATE_BUFFER  // allocate buffer on local heap for error text
                | FORMAT_MESSAGE_IGNORE_INSERTS   // Important! will fail otherwise, since we're not
                                                  // (and CANNOT) pass insertion parameters
            ,
            NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&errorText, 0, NULL);
        if (errorText != nullptr) {
            errorStr = std::wstring(static_cast<const wchar_t*>(errorText), outputSize + 1);
            LocalFree(errorText);  // release memory allocated by FormatMessage()
        }

        throw Exception(IVW_CONTEXT, "Failed to load library: {}\n Error ({}): {}", filePath_,
                        error, util::fromWstring(errorStr));
    }
#else
    // RTLD_GLOBAL gives all other loaded libraries access to library
    // RTLD_LOCAL is preferred but would require each module to
    // explicitly load its dependent libraries as well.
    handle_ = dlopen(filePath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if (!handle_) {
        throw Exception("Failed to load library: " + filePath, IVW_CONTEXT);
    }
#endif
}

SharedLibrary::SharedLibrary(SharedLibrary&& rhs) noexcept
    : filePath_{std::move(rhs.filePath_)}, handle_{std::exchange(rhs.handle_, nullptr)} {}

SharedLibrary& SharedLibrary::operator=(SharedLibrary&& that) noexcept {
    if (this != &that) {
#if WIN32
        FreeLibrary(handle_);
#else
        dlclose(handle_);
#endif
        handle_ = std::exchange(that.handle_, nullptr);
        filePath_ = std::exchange(that.filePath_, "");
    }
    return *this;
}

SharedLibrary::~SharedLibrary() {
    if (handle_) {
#if WIN32
        FreeLibrary(handle_);
#else
        dlclose(handle_);
#endif
    }
}

void* SharedLibrary::findSymbol(const std::string& name) {
#if WIN32
    return GetProcAddress(handle_, name.c_str());
#else
    return dlsym(handle_, name.c_str());
#endif
}

std::set<std::string> SharedLibrary::libraryFileExtensions() {
#if WIN32
    return {"dll"};
#else
    return {"so", "dylib", "bundle"};
#endif
}

void SharedLibrary::release() { handle_ = nullptr; }

namespace util {

std::vector<std::string> getLibrarySearchPaths() {
    return std::vector<std::string>{filesystem::getInviwoBinDir(),
                                    filesystem::getPath(inviwo::PathType::Modules)};
}

}  // namespace util

}  // namespace inviwo
