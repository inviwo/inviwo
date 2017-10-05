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

#ifndef IVW_SHAREDLIBRARY_H
#define IVW_SHAREDLIBRARY_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

#if WIN32
// Forward declare HINSTANCE
#ifndef _WINDEF_
struct HINSTANCE__; // Forward or never
typedef HINSTANCE__* HINSTANCE;
#endif
#endif

namespace inviwo {

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
    virtual ~SharedLibrary();

    std::string getFilePath() { return filePath_; }

    /** 
     * \brief Get function address from library.
     *
     * Example usage:
     * @code
     *      typedef InviwoModuleFactoryObject* (__stdcall *f_getModule)();
     *      f_getModule moduleFunc = (f_getModule)sharedLib->findSymbol("createModule");
     * @endcode
     * @param std::string name Function name
     * @return void* Address to function if found, otherwise nullptr
     */
    virtual void* findSymbol(const std::string& name);
private:
    std::string filePath_;
#if WIN32
    HINSTANCE handle_ = nullptr;
#else
    void* handle_ = nullptr;
#endif
};

} // namespace

#endif // IVW_SHAREDLIBRARY_H

