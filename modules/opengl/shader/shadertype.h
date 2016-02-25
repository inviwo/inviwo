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

#ifndef IVW_SHADERTYPE_H
#define IVW_SHADERTYPE_H

#include <modules/opengl/openglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/opengl/inviwoopengl.h>

namespace inviwo {

/**
 * \class ShaderType
 * \brief Encapsulate a GLenum shader type, and related information.
 */
class IVW_MODULE_OPENGL_API ShaderType { 
public:
    ShaderType() = default;
    explicit ShaderType(GLenum type);
    ShaderType(const ShaderType&) = default;
    ShaderType& operator=(const ShaderType&) = default;
    ~ShaderType() = default;

    operator GLenum() const;
    operator bool() const;

    std::string extension() const;

    static ShaderType Vertex;
    static ShaderType Geometry;
    static ShaderType Fragment;
    static ShaderType TessellationControl;
    static ShaderType TessellationEvaluation;
    static ShaderType Compute;

    static std::string extension(const ShaderType& type);
    static ShaderType get(const std::string& ext);

private:
    GLenum type_ = 0;
};

bool operator==(const ShaderType& lhs, const ShaderType& rhs);

} // namespace

namespace std {
template<>
struct hash<inviwo::ShaderType> {
    size_t operator()(const inviwo::ShaderType& type) const {
        return std::hash<GLenum>()(static_cast<GLenum>(type));
    }
};
}  // namespace

#endif // IVW_SHADERTYPE_H

