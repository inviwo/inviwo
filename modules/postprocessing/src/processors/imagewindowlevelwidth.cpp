/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2018 Inviwo Foundation
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

#include <modules/postprocessing/processors/imagewindowlevelwidth.h>
#include <modules/opengl/shader/shaderutils.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/interaction/events/mouseevent.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ImageWindowLevelWidth::processorInfo_{
    "org.inviwo.ImageWindowLevelWidth",  // Class identifier
    "Image Window Level and Width",      // Display name
    "Image Operation",                   // Category
    CodeState::Experimental,             // Code state
    Tags::None,                          // Tags
};
const ProcessorInfo ImageWindowLevelWidth::getProcessorInfo() const { return processorInfo_; }

ImageWindowLevelWidth::ImageWindowLevelWidth()
    : ImageGLProcessor("windowlevelwidth.frag")
    , volume_("volume")
    , windowCenter_("window_center", "Window Center", 0.5f, 0.0f, 1.0f, 0.01f)
    , windowWidth_("window_width", "Window Width", 1.0f, 0.0f, 1.0f, 0.01f)
    , sensitivity_("sensitivity", "Sensitivity", 1.0f, 0.1f, 10.0f, 0.01f)
    , mouseEventWindowCenter_("mouseEventWindowCenter_", "Mouse: Window Center",
                              [this](Event* e) { windowCenterCallback(e); }, MouseButton::Left,
                              MouseState::Press | MouseState::Move, KeyModifier::Shift)
    , mouseEventWindowWidth_("mouseEventWindowWidth_", "Mouse: Window Width",
                              [this](Event* e) { windowWidthCallback(e); }, MouseButton::Left,
                              MouseState::Press | MouseState::Move, KeyModifier::Control) {
    addPort(volume_);

    addProperty(windowCenter_);
    addProperty(windowWidth_);

    addProperty(mouseEventWindowCenter_);
    addProperty(mouseEventWindowWidth_);
}

void ImageWindowLevelWidth::preProcess(TextureUnitContainer&) {
    const auto volume = volume_.getData();
    const auto& datamap = volume->dataMap_;

    /*
    const auto value = volumeRAM->getAsDouble(pt); // in data range
    const auto value_HU = value * datamap.rescaleSlope + datamap.rescaleIntercept; // in houndsfield units
    const auto value_normalized = (value_HU - windowMin) / windowWidth;
    const auto value_clamped = glm::clamp(value_normalized, 0.0, 1.0);
    */

    utilgl::setUniforms(shader_, windowCenter_, windowWidth_);
}

void ImageWindowLevelWidth::windowCenterCallback(Event* event) {
    const auto mouseEvent = static_cast<MouseEvent*>(event);
    const auto mousePos = vec2(mouseEvent->posNormalized());

    if (mouseEvent->state() == MouseState::Move) {
        const auto mousePosDiff = mousePos - lastMousePos_;
        windowCenter_ = glm::clamp(windowCenter_.get() + sensitivity_ * mousePosDiff.x, 0.0f, 1.0f);
    }

    lastMousePos_ = mousePos;
    event->markAsUsed();
}

void ImageWindowLevelWidth::windowWidthCallback(Event* event) {
    const auto dims = vec2(outport_.getDimensions());
    const auto aspect_ratio = dims.x / dims.y;

    LogInfo(dims);

    const auto mouseEvent = static_cast<MouseEvent*>(event);
    const auto mousePos = vec2(mouseEvent->posNormalized());

    if (mouseEvent->state() == MouseState::Move) {
        const auto mousePosDiff = mousePos - lastMousePos_;
        windowWidth_ =
            glm::clamp(windowWidth_.get() + aspect_ratio * sensitivity_ * mousePosDiff.y, 0.0f, 1.0f);
    }

    lastMousePos_ = mousePos;
    event->markAsUsed();
}

}  // namespace inviwo
