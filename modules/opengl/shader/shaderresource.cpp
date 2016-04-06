/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/stdextensions.h>
#include <modules/opengl/shader/shaderresource.h>

namespace inviwo {

FileShaderResource::FileShaderResource(const std::string& key, const std::string& fileName)
    : key_(key), fileName_(fileName) {
    InviwoApplication::getPtr()->registerFileObserver(this);
    startFileObservation(fileName_);
}

FileShaderResource::~FileShaderResource() {
    stopFileObservation(fileName_);
    InviwoApplication::getPtr()->unRegisterFileObserver(this);
}

std::unique_ptr<ShaderResource> FileShaderResource::clone() {
    return util::make_unique<FileShaderResource>(key_, fileName_);
}

std::string FileShaderResource::key() const { return key_; }

std::string FileShaderResource::source() const {
    if (!cache_.empty()) return cache_;
    std::ifstream stream(fileName_);
    std::stringstream buffer;
    buffer << stream.rdbuf();
    cache_ = buffer.str();
    return cache_;
}

std::string FileShaderResource::file() const { return fileName_; }

void FileShaderResource::fileChanged(const std::string& fileName) { 
    cache_ = "";
    callbacks_.invoke(this); 
}

StringShaderResource::StringShaderResource(const std::string& key, const std::string& source)
    : key_(key), source_(source) {}

std::unique_ptr<ShaderResource> StringShaderResource::clone() {
    return util::make_unique<StringShaderResource>(key_, source_);
}

std::string StringShaderResource::key() const { return key_; }

std::string StringShaderResource::source() const { return source_; }

void StringShaderResource::setSource(const std::string& source) {
    source_ = source;
    callbacks_.invoke(this);
}

}  // namespace
