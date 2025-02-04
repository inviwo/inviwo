/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2025 Inviwo Foundation
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

#include <modules/opengl/shader/fileshaderresource.h>

#include <inviwo/core/common/inviwoapplicationutil.h>

#include <iostream>
#include <sstream>

namespace inviwo {
FileShaderResource::FileShaderResource(std::string_view key, const std::filesystem::path& fileName)
    : FileObserver(util::getInviwoApplication()), key_(key), fileName_(fileName) {
    startFileObservation(fileName_);
}

std::unique_ptr<ShaderResource> FileShaderResource::clone() const {
    return std::make_unique<FileShaderResource>(key_, fileName_);
}

const std::string& FileShaderResource::key() const { return key_; }

const std::string& FileShaderResource::source() const {
    if (!cache_.empty()) return cache_;
    auto stream = std::ifstream(fileName_);
    std::stringstream buffer;
    buffer << stream.rdbuf();
    cache_ = std::move(buffer).str();
    return cache_;
}

void FileShaderResource::setSource(std::string_view source) {
    auto file = std::ofstream(fileName_);
    file << source;
    file.close();
}

const std::filesystem::path& FileShaderResource::file() const { return fileName_; }

void FileShaderResource::fileChanged(const std::filesystem::path& /*fileName*/) {
    cache_ = "";
    callbacks_.invoke(this);
}

}  // namespace inviwo
