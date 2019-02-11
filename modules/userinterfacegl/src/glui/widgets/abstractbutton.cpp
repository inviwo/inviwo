/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#include <modules/userinterfacegl/glui/widgets/abstractbutton.h>
#include <modules/userinterfacegl/glui/renderer.h>

#include <inviwo/core/util/moduleutils.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/texture2darray.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/rendering/meshdrawergl.h>

#include <vector>

namespace inviwo {

namespace glui {

AbstractButton::AbstractButton(const std::string &label, Processor &processor, Renderer &uiRenderer,
                               const ivec2 &extent)
    : Element(label, processor, uiRenderer) {
    widgetExtent_ = extent;
    setLabelBold(true);

    std::vector<std::string> btnFiles = {"button-normal.png", "button-pressed.png",
                                         "button-checked.png", "button-halo.png",
                                         "button-border.png"};
    // normal, pressed, checked, corresponding halo (3x) and border (3x)
    uiTextureMap_ = {{0, 1, 2, 3, 3, 3, 4, 4, 4}};
    uiTextures_ = uiRenderer_->createUITextures(
        "button", btnFiles, module::getModulePath("UserInterfaceGL", ModulePath::Images));
}

ivec2 AbstractButton::computeLabelPos(int descent) const {
    // align label to be vertically and horizontally centered within the button
    if (glm::all(glm::greaterThan(labelExtent_, ivec2(0)))) {
        vec2 labelSize(labelExtent_);
        labelSize.y -= descent;
        ivec2 labelOrigin(ivec2(glm::floor(vec2(getWidgetExtentScaled()) * 0.5f + 0.5f)));
        // compute offset for vertical alignment in the center
        vec2 labelOffset(-labelSize.x * 0.5f, -labelSize.y * 0.5f);

        return ivec2(labelOrigin + ivec2(labelOffset + 0.5f));
    }
    return ivec2(0);
}

Element::UIState AbstractButton::uiState() const {
    return (checked_ ? UIState::Checked : pushed_ ? UIState::Pressed : UIState::Normal);
}

vec2 AbstractButton::marginScale() const {
    if (uiTextures_) {
        // use unscaled widgetExtent_ here so that corners are also scaled along with the widget
        return (vec2(uiTextures_->getDimensions()) / vec2(widgetExtent_));
    }
    return vec2(1.0f);
}

}  // namespace glui

}  // namespace inviwo
