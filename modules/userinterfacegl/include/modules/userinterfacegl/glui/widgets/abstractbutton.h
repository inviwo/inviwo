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

#pragma once

#include <modules/userinterfacegl/userinterfaceglmoduledefine.h>  // for IVW_MODULE_USERINTERFAC...

#include <inviwo/core/util/glmvec.h>               // for ivec2, size2_t, vec2
#include <modules/userinterfacegl/glui/element.h>  // for Element, Element::UIState

#include <array>   // for array
#include <string>  // for string

namespace inviwo {

class Processor;
class Texture2DArray;

namespace glui {

class Renderer;

/**
 * \brief glui::Element representing a generic button
 */
class IVW_MODULE_USERINTERFACEGL_API AbstractButton : public Element {
public:
    AbstractButton(const std::string& label, Processor& processor, Renderer& uiRenderer,
                   const ivec2& extent = ivec2(100, 24));
    virtual ~AbstractButton() = default;

protected:
    virtual void renderWidget(const ivec2& origin, const size2_t& canvasDim) override = 0;

    virtual ivec2 computeLabelPos(int descent) const override;
    virtual UIState uiState() const override;
    virtual vec2 marginScale() const override;

    Texture2DArray* uiTextures_;
    std::array<int, 9> uiTextureMap_;
};

}  // namespace glui

}  // namespace inviwo
