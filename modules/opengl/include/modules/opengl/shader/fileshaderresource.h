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
#pragma once

#include <modules/opengl/openglmoduledefine.h>

#include <modules/opengl/shader/shaderresource.h>
#include <inviwo/core/util/fileobserver.h>

namespace inviwo {

class IVW_MODULE_OPENGL_API FileShaderResource : public ShaderResource, public FileObserver {
public:
    FileShaderResource(std::string_view key, const std::filesystem::path& fileName);   
    FileShaderResource(const FileShaderResource& rhs) = delete;
    FileShaderResource(FileShaderResource&& rhs) noexcept = default;
    FileShaderResource& operator=(const FileShaderResource&) = delete;
    FileShaderResource& operator=(FileShaderResource&&) noexcept = default;
    virtual ~FileShaderResource() = default;
    virtual std::unique_ptr<ShaderResource> clone() const override;

    virtual const std::string& key() const override;
    virtual const std::string& source() const override;

    virtual void setSource(std::string_view source) override;

    const std::filesystem::path& file() const;

    virtual void fileChanged(const std::filesystem::path& fileName) override;

private:
    std::string key_;
    std::filesystem::path fileName_;

    mutable std::string cache_;
};

}  // namespace inviwo
