/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2024 Inviwo Foundation
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

#include <modules/userinterfacegl/glui/widgets/checkbox.h>

#include <inviwo/core/common/inviwomodule.h>        // for ModulePath, ModulePath::Images
#include <inviwo/core/interaction/pickingmapper.h>  // for PickingMapper
#include <inviwo/core/util/glmvec.h>                // for ivec2, vec2, size2_t
#include <inviwo/core/util/logcentral.h>            // for LogCentral
#include <inviwo/core/util/moduleutils.h>           // for getModulePath
#include <modules/opengl/rendering/meshdrawergl.h>  // for MeshDrawerGL
#include <modules/opengl/shader/shader.h>           // for Shader
#include <modules/opengl/texture/texture2darray.h>  // for Texture2DArray
#include <modules/opengl/texture/textureunit.h>     // for TextureUnit
#include <modules/userinterfacegl/glui/element.h>   // for Element::UIState, Element::UIState::C...
#include <modules/userinterfacegl/glui/renderer.h>  // for Renderer

#include <functional>  // for __base, function
#include <ostream>     // for operator<<, basic_ostream
#include <vector>      // for vector

#include <glm/vec2.hpp>               // for operator+, vec<>::(anonymous), vec
#include <glm/vector_relational.hpp>  // for all, greaterThan

namespace inviwo {
class Processor;

namespace glui {

std::string_view CheckBox::getClassIdentifier() const { return classIdentifier; }

CheckBox::CheckBox(const std::string& label, Processor& processor, Renderer& uiRenderer,
                   const ivec2& extent)
    : Element(label, processor, uiRenderer) {
    widgetExtent_ = extent;
    action_ = [&]() { log::user::info("UI checkbox {} toggled: {}", getLabel(), getValue()); };

    std::vector<std::filesystem::path> textureFiles = {
        "checkbox-fill.png", "checkbox-unchecked.png", "checkbox-checked.png",
        "checkbox-unchecked-halo.png", "checkbox-checked-halo.png"};
    uiTextures_ = uiRenderer_->createUITextures(
        "checkbox", textureFiles, util::getModulePath("UserInterfaceGL", ModulePath::Images));

    // for a checkbox, the main UI components are stored in the border
    // unchecked, checked, checked, corresponding halo (3x) and border (3x)
    uiTextureMap_ = {{0, 0, 0, 3, 4, 4, 1, 2, 2}};
}

void CheckBox::renderWidget(const ivec2& origin, const size2_t&) {
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

void CheckBox::setValue(bool value) { checked_ = value; }

bool CheckBox::getValue() const { return checked_; }

ivec2 CheckBox::computeLabelPos(int descent) const {
    const int labelSpacing = 5;

    if (glm::all(glm::greaterThan(labelExtent_, ivec2(0)))) {
        vec2 labelSize(labelExtent_);
        labelSize.y -= descent;
        const ivec2 extent(getWidgetExtentScaled());
        ivec2 labelOrigin(extent.x + labelSpacing, extent.y / 2);
        // compute offset for vertical alignment in the center
        // add 1 pixel vertically since the texture is not centered
        vec2 labelOffset(0.0f, -labelSize.y * 0.5f - 1.0f);
        return ivec2(labelOrigin + ivec2(labelOffset + 0.5f));
    }
    return ivec2(0);
}

Element::UIState CheckBox::uiState() const {
    auto state = (checked_ ? UIState::Checked : UIState::Normal);
    if (pushed_) {
        // invert state when pressed
        if (state == UIState::Checked) {
            state = UIState::Normal;
        } else {
            state = UIState::Checked;
        }
    }
    return state;
}

void CheckBox::updateState() { checked_ = !checked_; }

}  // namespace glui

}  // namespace inviwo
