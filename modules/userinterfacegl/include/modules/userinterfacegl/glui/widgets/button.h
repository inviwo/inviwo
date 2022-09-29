/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2022 Inviwo Foundation
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

#include <inviwo/core/util/glmvec.h>                              // for ivec2, size2_t
#include <modules/userinterfacegl/glui/widgets/abstractbutton.h>  // for AbstractButton

#include <string>                                                 // for string

namespace inviwo {
class Processor;

namespace glui {

class Renderer;

/**
 * \class Button
 * \brief glui::Element representing a button with the label centered within
 */
class IVW_MODULE_USERINTERFACEGL_API Button : public AbstractButton {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    Button(const std::string& label, Processor& processor, Renderer& uiRenderer,
           const ivec2& extent = ivec2(100, 24));
    virtual ~Button() = default;

protected:
    virtual void renderWidget(const ivec2& origin, const size2_t& canvasDim) override;
};

}  // namespace glui

}  // namespace inviwo
