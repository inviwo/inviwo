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
#ifdef WIN32
    , handle_(nullptr)
#else 
    , handle_(nullptr)
#endif
{
#if WIN32
    handle_ = LoadLibraryA(filePath_.c_str());
    if (!handle_) {
        throw Exception("Failed to load library: " + filePath);
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
    ivwAssert(depends_.size() == dependenciesVersion_.size(), "Each module dependency must have a version");
}

InviwoModuleFactoryObject::~InviwoModuleFactoryObject() {
    
}

} // namespace

