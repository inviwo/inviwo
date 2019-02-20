/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifndef IVW_SHAREDLIBRARY_H
#define IVW_SHAREDLIBRARY_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

#include <set>
#include <vector>
#include <string>

#if WIN32
// Forward declare HINSTANCE
#ifndef _WINDEF_
struct HINSTANCE__;  // Forward or never
typedef HINSTANCE__* HINSTANCE;
#endif
#endif

namespace inviwo {

namespace util {

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
IVW_CORE_API std::vector<std::string> getLibrarySearchPaths();

IVW_CORE_API bool hasAddLibrarySearchDirsFunction();
IVW_CORE_API std::vector<void*> addLibrarySearchDirs(const std::vector<std::string>& dirs);
IVW_CORE_API void removeLibrarySearchDirs(const std::vector<void*>& dirs);

IVW_CORE_API std::vector<std::string> getLoadedLibraries();
}  // namespace util

class IVW_CORE_API LibrarySearchDirs {
public:
    LibrarySearchDirs(const std::vector<std::string>& dirs = {});
    void add(const std::vector<std::string>& dirs);
    ~LibrarySearchDirs();

private:
    std::vector<void*> addedDirs_;
};

/**
 * \class SharedLibrary
 * \brief Loader for dll/so/dylib. Get functions from loaded library using findSymbol(...).
 *
 * Loads specified dll/so/dylib on construction and unloads it on destruction.
 * Throws an inviwo::Exception if library failed to load.
 */
class IVW_CORE_API SharedLibrary {
public:
    SharedLibrary(const std::string& filePath);
    SharedLibrary(const SharedLibrary& rhs) = delete;
    SharedLibrary& operator=(const SharedLibrary& that) = delete;
    SharedLibrary(SharedLibrary&& rhs);
    SharedLibrary& operator=(SharedLibrary&& that);
    ~SharedLibrary();

    std::string getFilePath() { return filePath_; }

    /**
     * \brief Get function address from library.
     *
     * Example usage:
     * \code{.cpp}
     *      using f_getModule = InviwoModuleFactoryObject* (__stdcall *)();
     *      auto moduleFunc = reinterpret_cast<f_getModule>(sharedLib->findSymbol("createModule"));
     * \endcode
     * @param name Function name
     * @return Address to function if found, otherwise nullptr
     */
    void* findSymbol(const std::string& name);

    /**
     * \brief Get typed function address from library.
     *
     * Example usage:
     * \code{.cpp}
     *      using f_getModule = InviwoModuleFactoryObject* (__stdcall *)();
     *      auto moduleFunc = sharedLib->findSymbolTyped<f_getModule>("createModule"));
     * \endcode
     * @param name Function name
     * @return Address to function if found, otherwise nullptr
     */
    template <typename T>
    T findSymbolTyped(const std::string& name);

    static std::set<std::string> libraryFileExtensions();

    /**
     * Reset the handle and effectively leak the lib.
     * Needed for some dll that crashes on exit otherwise
     */
    void release();

private:
    std::string filePath_;
#if WIN32
    HINSTANCE handle_ = nullptr;
#else
    void* handle_ = nullptr;
#endif
};

template <typename T>
T SharedLibrary::findSymbolTyped(const std::string& name) {
    return reinterpret_cast<T>(findSymbol(name));
}

}  // namespace inviwo

#endif  // IVW_SHAREDLIBRARY_H
