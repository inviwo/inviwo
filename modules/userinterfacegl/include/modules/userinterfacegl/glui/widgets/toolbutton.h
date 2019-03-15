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

#ifndef IVW_GLUITOOLBUTTON_H
#define IVW_GLUITOOLBUTTON_H

#include <modules/userinterfacegl/userinterfaceglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/userinterfacegl/glui/widgets/abstractbutton.h>
#include <modules/opengl/rendering/texturequadrenderer.h>

namespace inviwo {

class Texture2DArray;
class Texture2D;

namespace glui {

class Renderer;

/**
 * \class ToolButton
 * \brief glui::Element representing a tool button with an image instead of a label
 */
class IVW_MODULE_USERINTERFACEGL_API ToolButton : public AbstractButton {
public:
    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    ToolButton(const std::string &filename, Processor &processor, Renderer &uiRenderer,
               const ivec2 &extent = ivec2(24, 24));
    ToolButton(std::shared_ptr<Texture2D> labelImage, Processor &processor, Renderer &uiRenderer,
               const ivec2 &extent = ivec2(24, 24));
    virtual ~ToolButton() = default;

    void setImage(const std::string &filename);
    void setImage(std::shared_ptr<Texture2D> texture);

    void setMargins(int top, int left, int bottom, int right);
    const ivec4 &getMargins() const;

private:
    virtual void renderWidget(const ivec2 &origin, const size2_t &canvasDim) override;

    static std::shared_ptr<Texture2D> loadImage(const std::string &filename);

    std::shared_ptr<Texture2D> labelImage_;
    ivec4 margins_ = ivec4(6, 6, 6, 6);  //!< top, left, bottom, right

    TextureQuadRenderer quadRenderer_;
};

}  // namespace glui

}  // namespace inviwo

#endif  // IVW_GLUITOOLBUTTON_H
