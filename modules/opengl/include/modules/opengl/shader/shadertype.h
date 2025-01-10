/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2025 Inviwo Foundation
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

#include <modules/opengl/openglmoduledefine.h>  // for IVW_MODULE_OPENGL_API

#include <modules/opengl/inviwoopengl.h>  // for GLenum

#include <cstddef>     // for size_t
#include <functional>  // for hash
#include <memory>      // for hash
#include <string>      // for string
#include <string_view>
#include <filesystem>

namespace inviwo {

/**
 * \class ShaderType
 * \brief Encapsulate a GLenum shader type, and related information.
 */
class IVW_MODULE_OPENGL_API ShaderType {
public:
    explicit constexpr ShaderType(GLenum type) : type_(type) {}

    operator GLenum() const;
    operator bool() const;

    std::string extension() const;
    std::string name() const;

    static std::string extension(const ShaderType& type);
    static ShaderType typeFromExtension(std::string_view ext);
    static ShaderType typeFromString(std::string_view str);
    static ShaderType typeFromFile(const std::filesystem::path& ext);

    inline friend bool operator==(const ShaderType& lhs, const ShaderType& rhs) {
        return lhs.type_ == rhs.type_;
    }

    static const ShaderType Vertex;
    static const ShaderType Geometry;
    static const ShaderType Fragment;
    static const ShaderType TessellationControl;
    static const ShaderType TessellationEvaluation;
    static const ShaderType Compute;

private:
    GLenum type_ = 0;
};

}  // namespace inviwo

namespace std {
template <>
struct hash<inviwo::ShaderType> {
    size_t operator()(const inviwo::ShaderType& type) const {
        return std::hash<GLenum>()(static_cast<GLenum>(type));
    }
};
}  // namespace std
