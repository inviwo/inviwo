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

#ifndef IVW_GLUICHECKBOX_H
#define IVW_GLUICHECKBOX_H

#include <modules/userinterfacegl/userinterfaceglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/userinterfacegl/glui/element.h>

namespace inviwo {

class Texture2DArray;

namespace glui {

class Renderer;

/**
 * \brief glui::Element representing a checkbox with the label positioned on the right side
 */
class IVW_MODULE_USERINTERFACEGL_API CheckBox : public Element {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    CheckBox(const std::string &label, Processor &processor, Renderer &uiRenderer,
             const ivec2 &extent = ivec2(24, 24));
    virtual ~CheckBox() = default;

    void setValue(bool value);
    bool getValue() const;

private:
    virtual void renderWidget(const ivec2 &origin, const size2_t &canvasDim) override;

    virtual ivec2 computeLabelPos(int descent) const override;
    virtual UIState uiState() const override;
    virtual void updateState() override;

    Texture2DArray *uiTextures_;
    std::array<int, 9> uiTextureMap_;
};

}  // namespace glui

}  // namespace inviwo

#endif  // IVW_GLUICHECKBOX_H
