/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2023 Inviwo Foundation
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

#include <modules/opengl/shader/shadertype.h>

#include <array>

namespace inviwo {

ShaderType::operator GLenum() const { return type_; }

std::string ShaderType::extension() const { return extension(*this); }

std::string ShaderType::extension(const ShaderType& type) {
    // Following https://www.khronos.org/opengles/sdk/tools/Reference-Compiler/
    switch (type.type_) {
        case GL_VERTEX_SHADER:
            return ".vert";
        case GL_GEOMETRY_SHADER:
            return ".geom";
        case GL_FRAGMENT_SHADER:
            return ".frag";
        case GL_TESS_CONTROL_SHADER:
            return ".tesc";
        case GL_TESS_EVALUATION_SHADER:
            return ".tese";
        case GL_COMPUTE_SHADER:
            return ".comp";
        default:
            return "";
    }
}

std::string ShaderType::name() const {
    switch (type_) {
        case GL_VERTEX_SHADER:
            return "vertex";
        case GL_GEOMETRY_SHADER:
            return "geometry";
        case GL_FRAGMENT_SHADER:
            return "fragment";
        case GL_TESS_CONTROL_SHADER:
            return "tesc_control";
        case GL_TESS_EVALUATION_SHADER:
            return "tese_evaluation";
        case GL_COMPUTE_SHADER:
            return "compute";
        default:
            return "invalid";
    }
}

const ShaderType ShaderType::Vertex{GL_VERTEX_SHADER};
const ShaderType ShaderType::Geometry{GL_GEOMETRY_SHADER};
const ShaderType ShaderType::Fragment{GL_FRAGMENT_SHADER};
const ShaderType ShaderType::TessellationControl{GL_TESS_CONTROL_SHADER};
const ShaderType ShaderType::TessellationEvaluation{GL_TESS_EVALUATION_SHADER};
const ShaderType ShaderType::Compute{GL_COMPUTE_SHADER};

ShaderType::operator bool() const {
    return type_ == GL_VERTEX_SHADER || type_ == GL_GEOMETRY_SHADER ||
           type_ == GL_FRAGMENT_SHADER || type_ == GL_TESS_CONTROL_SHADER ||
           type_ == GL_TESS_EVALUATION_SHADER || type_ == GL_COMPUTE_SHADER;
}

namespace {
const std::array<std::pair<std::string_view, ShaderType>, 6> extToShaderType{
    {{".vert", ShaderType::Vertex},
     {".geom", ShaderType::Geometry},
     {".frag", ShaderType::Fragment},
     {".tesc", ShaderType::TessellationControl},
     {".tese", ShaderType::TessellationEvaluation},
     {".comp", ShaderType::Compute}}};
}

ShaderType ShaderType::typeFromExtension(std::string_view ext) {
    if (ext == "vert")
        return Vertex;
    else if (ext == "geom")
        return Geometry;
    else if (ext == "frag")
        return Fragment;
    else if (ext == "tesc")
        return TessellationControl;
    else if (ext == "tese")
        return TessellationEvaluation;
    else if (ext == "comp")
        return Compute;
    else
        return ShaderType(0);
}

ShaderType ShaderType::typeFromString(std::string_view str) {
    for (auto& [ext, type] : extToShaderType) {
        if (str.ends_with(ext)) return type;
    }
    return ShaderType(0);
}
ShaderType ShaderType::typeFromFile(const std::filesystem::path& file) {
    for (auto& [ext, type] : extToShaderType) {
        if (file.extension() == ext) return type;
    }
    return ShaderType(0);
}

}  // namespace inviwo
