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

#include <modules/userinterfacegl/glui/widgets/toolbutton.h>
#include <modules/userinterfacegl/glui/renderer.h>

#include <inviwo/core/util/moduleutils.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/image/layerram.h>

#include <modules/opengl/image/layergl.h>
#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/texture2d.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/rendering/meshdrawergl.h>

#include <vector>

namespace inviwo {

namespace glui {

const std::string ToolButton::classIdentifier = "org.inviwo.glui.ToolButton";
std::string ToolButton::getClassIdentifier() const { return classIdentifier; }

ToolButton::ToolButton(const std::string &filename, Processor &processor, Renderer &uiRenderer,
                       const ivec2 &extent)
    : AbstractButton("", processor, uiRenderer, extent)
    , labelImage_(loadImage(filename))
    , quadRenderer_(Shader("rendertexturequad.vert", "labelui.frag")) {
    setLabelVisible(false);
}

ToolButton::ToolButton(std::shared_ptr<Texture2D> labelImage, Processor &processor,
                       Renderer &uiRenderer, const ivec2 &extent)
    : AbstractButton("", processor, uiRenderer, extent)
    , labelImage_(labelImage)
    , quadRenderer_(Shader("rendertexturequad.vert", "labelui.frag")) {
    setLabelVisible(false);
}

void ToolButton::setImage(const std::string &filename) { labelImage_ = loadImage(filename); }

void ToolButton::setImage(std::shared_ptr<Texture2D> texture) { labelImage_ = texture; }

void ToolButton::setMargins(int top, int left, int bottom, int right) {
    margins_ = ivec4(top, left, bottom, right);
}

const ivec4 &ToolButton::getMargins() const { return margins_; }

void ToolButton::renderWidget(const ivec2 &origin, const size2_t &canvasDim) {
    TextureUnit texUnit;
    texUnit.activate();
    uiTextures_->bind();

    // bind textures
    auto &uiShader = uiRenderer_->getShader();
    uiShader.setUniform("arrayTexSampler", texUnit.getUnitNumber());

    uiShader.setUniform("origin", vec2(origin + widgetPos_));
    uiShader.setUniform("extent", vec2(getWidgetExtentScaled()));

    // set up picking color
    uiShader.setUniform("pickingColor", pickingMapper_.getColor(0));
    uiShader.setUniform("uiState", ivec2(uiState(), (hovered_ ? 1 : 0)));
    uiShader.setUniform("marginScale", marginScale());

    // render quad
    uiRenderer_->getMeshDrawer()->draw();

    // render button image
    if (labelImage_) {
        utilgl::BlendModeState blending(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        const ivec2 imagePos(origin + ivec2(margins_.y, margins_.z));
        const ivec2 imageExtent =
            getWidgetExtentScaled() - ivec2(margins_.y + margins_.w, margins_.x + margins_.z);

        auto &shader = quadRenderer_.getShader();
        shader.activate();
        vec4 color(uiRenderer_->getSecondaryUIColor());
        if (!isEnabled()) {
            color = adjustColor(color);
        }
        shader.setUniform("uiColor", color);
        quadRenderer_.renderToRect(labelImage_, imagePos, imageExtent, canvasDim);
    }
}

std::shared_ptr<Texture2D> ToolButton::loadImage(const std::string &filename) {
    auto ext = filesystem::getFileExtension(filename);
    auto factory = InviwoApplication::getPtr()->getDataReaderFactory();
    if (auto reader = factory->getReaderForTypeAndExtension<Layer>(ext)) {
        try {
            // try to load texture data from current file
            auto layer = reader->readData(filename);
            return layer->getRepresentation<LayerGL>()->getTexture();
        } catch (DataReaderException const &e) {
            util::log(e.getContext(),
                      "Could not load texture data: " + filename + ", " + e.getMessage(),
                      LogLevel::Error);
            return nullptr;
        }
    } else {
        LogErrorCustom("ToolButton::loadImage",
                       "Could not find a data reader for texture data (" << ext << ").");
        return nullptr;
    }
}

}  // namespace glui

}  // namespace inviwo
