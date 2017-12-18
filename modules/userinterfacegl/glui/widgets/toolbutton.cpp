/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#include <modules/opengl/texture/textureunit.h>
#include <modules/opengl/texture/texture2d.h>
#include <modules/opengl/texture/texture2darray.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderutils.h>
#include <modules/opengl/rendering/meshdrawergl.h>

#include <vector>

namespace inviwo {

namespace glui {

ToolButton::ToolButton(const std::string &filename, Processor &processor, Renderer &uiRenderer,
                       const ivec2 &extent)
    : AbstractButton("", processor, uiRenderer, extent), labelImage_(loadImage(filename)) {
    setLabelVisible(false);
}

ToolButton::ToolButton(std::shared_ptr<Texture2D> labelImage, Processor &processor,
                       Renderer &uiRenderer, const ivec2 &extent)
    : AbstractButton("", processor, uiRenderer, extent), labelImage_(labelImage) {
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
    uiShader.setUniform("extent", vec2(widgetExtent_));

    // set up picking color
    uiShader.setUniform("pickingColor", pickingMapper_.getColor(0));
    uiShader.setUniform("uiState", ivec2(uiState(), (hovered_ ? 1 : 0)));
    uiShader.setUniform("marginScale", marginScale());

    // render quad
    uiRenderer_->getMeshDrawer()->draw();

    // render button image
    if (labelImage_) {
        const ivec2 imagePos(origin + ivec2(margins_.y, margins_.z));
        const ivec2 imageExtent =
            widgetExtent_ - ivec2(margins_.y + margins_.w, margins_.x + margins_.z);
        uiRenderer_->getTextureQuadRenderer().renderToRect(labelImage_, imagePos, imageExtent,
                                                           canvasDim);
    }
}

std::shared_ptr<Texture2D> ToolButton::loadImage(const std::string &filename) {
    auto ext = filesystem::getFileExtension(filename);
    auto factory = InviwoApplication::getPtr()->getDataReaderFactory();
    if (auto reader = factory->getReaderForTypeAndExtension<Layer>(ext)) {
        std::shared_ptr<Layer> layer;
        // try to load texture data from current file
        try {
            layer = reader->readData(filename);
            auto layerRAM = layer->getRepresentation<LayerRAM>();
            // Hack needs to set format here since LayerDisk does not have a format.
            layer->setDataFormat(layerRAM->getDataFormat());

            auto texture = std::make_shared<Texture2D>(
                layer->getDimensions(), GLFormats::get(layer->getDataFormat()->getId()), GL_LINEAR);
            texture->initialize(layerRAM->getData());
            return texture;
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
