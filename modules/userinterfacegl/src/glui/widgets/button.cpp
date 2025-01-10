/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2025 Inviwo Foundation
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

#include <modules/userinterfacegl/glui/widgets/button.h>

#include <inviwo/core/interaction/pickingmapper.h>                // for PickingMapper
#include <inviwo/core/util/glmvec.h>                              // for ivec2, vec2, size2_t
#include <modules/opengl/rendering/meshdrawergl.h>                // for MeshDrawerGL
#include <modules/opengl/shader/shader.h>                         // for Shader
#include <modules/opengl/texture/texture2darray.h>                // for Texture2DArray
#include <modules/opengl/texture/textureunit.h>                   // for TextureUnit
#include <modules/userinterfacegl/glui/renderer.h>                // for Renderer
#include <modules/userinterfacegl/glui/widgets/abstractbutton.h>  // for AbstractButton

#include <array>  // for array

#include <glm/vec2.hpp>  // for operator+

namespace inviwo {
class Processor;

namespace glui {

std::string_view Button::getClassIdentifier() const { return classIdentifier; }

Button::Button(const std::string& label, Processor& processor, Renderer& uiRenderer,
               const ivec2& extent)
    : AbstractButton(label, processor, uiRenderer, extent) {}

void Button::renderWidget(const ivec2& origin, const size2_t&) {
    TextureUnit texUnit;
    texUnit.activate();
    uiTextures_->bind();

    // bind textures
    auto& uiShader = uiRenderer_->getShader();
    uiShader.setUniform("arrayTexSampler", texUnit.getUnitNumber());
    uiShader.setUniform("arrayTexMap", uiTextureMap_);

    uiShader.setUniform("origin", vec2(origin + widgetPos_));
    uiShader.setUniform("extent", vec2(getWidgetExtentScaled()));

    // set up picking color
    uiShader.setUniform("pickingColor", pickingMapper_.getColor(0));
    uiShader.setUniform("uiState", ivec2(uiState(), (hovered_ ? 1 : 0)));
    uiShader.setUniform("marginScale", marginScale());

    // render quad
    uiRenderer_->getMeshDrawer()->draw();
}

}  // namespace glui

}  // namespace inviwo
