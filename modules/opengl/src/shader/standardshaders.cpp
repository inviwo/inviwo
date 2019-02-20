/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <modules/opengl/shader/standardshaders.h>

namespace inviwo {

namespace utilgl {

std::pair<ShaderType, std::shared_ptr<const ShaderResource>> imgIdentityVert() {
    static const std::shared_ptr<const ShaderResource> vert =
        std::make_shared<StringShaderResource>("image_identity.vert", R"(
out vec4 color_;
out vec3 texCoord_;

void main() {
    color_ = in_Color;
    texCoord_ = in_TexCoord;
    gl_Position = in_Vertex;
}
)");

    return {ShaderType::Vertex, vert};
}

std::pair<ShaderType, std::shared_ptr<const ShaderResource>> imgQuadVert() {
    static const std::shared_ptr<const ShaderResource> vert =
        std::make_shared<StringShaderResource>("image_quad.vert", R"(
out vec3 texCoord_;

void main() {
    texCoord_ = in_TexCoord;
    gl_Position = in_Vertex;
}
)");

    return {ShaderType::Vertex, vert};
}

std::pair<ShaderType, std::shared_ptr<const ShaderResource>> imgQuadFrag() {
    static const std::shared_ptr<const ShaderResource> frag =
        std::make_shared<StringShaderResource>("image_quad.frag", R"(
uniform sampler2D tex_;

in vec3 texCoord_;

void main() {
    FragData0 = texture(tex_, texCoord_.xy);
}
)");

    return {ShaderType::Fragment, frag};
}

}  // namespace utilgl

}  // namespace inviwo
