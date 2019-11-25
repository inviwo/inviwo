/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#include <modules/basegl/processors/imageprocessing/imagesubsetgl.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/interaction/events/gestureevent.h>
#include <inviwo/core/interaction/events/wheelevent.h>
#include <inviwo/core/network/networklock.h>

#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/texture/textureutils.h>

namespace inviwo {

const ProcessorInfo ImageSubsetGL::processorInfo_{
    "org.inviwo.ImageSubsetGL",  // Class identifier
    "Image Subset",              // Display name
    "Image Operation",           // Category
    CodeState::Stable,           // Code state
    Tags::GL,                    // Tags
};
const ProcessorInfo ImageSubsetGL::getProcessorInfo() const { return processorInfo_; }

ImageSubsetGL::ImageSubsetGL()
    : Processor()
    // Inport must use its connected outport's size
    , inport_("inputImage", true)
    // Outport shall not handle resize events
    , outport_("outputImage", false)
    , rangeX_("rangeX", "X Slices", 0, 256, 0, 256, 1, 1)
    , rangeY_("rangeY", "Y Slices", 0, 256, 0, 256, 1, 1)
    , shader_("img_texturequad.vert", "img_copy.frag")
    , rect_(DrawType::Triangles, ConnectivityType::Strip,
            {{{-1.0f, -1.0f}, {0.f, 0.f}},
             {{1.0f, -1.0f}, {1.f, 0.f}},
             {{-1.0f, 1.0f}, {0.f, 1.f}},
             {{1.0f, 1.0f}, {1.f, 1.f}}},
            {0u, 1u, 2u, 3u}) {
    addPort(inport_);
    addPort(outport_);
    addProperties(rangeX_, rangeY_);

    // Since the ranges depend on the input image dimensions, we make sure to always
    // serialize them so we can do a proper renormalization when we load new data.
    rangeX_.setSerializationMode(PropertySerializationMode::All);
    rangeY_.setSerializationMode(PropertySerializationMode::All);

    inport_.onChange([this]() {
        NetworkLock lock(this);

        // Update to the new dimensions.
        auto dims = inport_.getData()->getDimensions();

        rangeX_.setRangeNormalized(size2_t(0, dims.x));
        rangeY_.setRangeNormalized(size2_t(0, dims.y));

        // set the new dimensions to default if we were to press reset
        rangeX_.setCurrentStateAsDefault();
        rangeY_.setCurrentStateAsDefault();
    });
}

ImageSubsetGL::~ImageSubsetGL() = default;

void ImageSubsetGL::process() {
    const ivec2 offset{rangeX_.get().x, rangeY_.get().x};
    const ivec2 dim = ivec2{rangeX_.get().y, rangeY_.get().y} - offset;
    auto inputDim = inport_.getData()->getDimensions();
    if (dim == ivec2(inputDim))
        outport_.setData(inport_.getData());
    else {
        auto image = inport_.getData();
        auto outImage = std::make_shared<Image>(dim, image->getDataFormat());
        // pass meta data on
        outImage->copyMetaDataFrom(*image);

        outport_.setData(outImage);

        TextureUnit::setZeroUnit();

        TextureUnit colorUnit, depthUnit, pickingUnit;
        utilgl::activateAndClearTarget(outport_, ImageType::ColorDepthPicking);

        shader_.activate();
        shader_.setUniform("color_", colorUnit.getUnitNumber());
        shader_.setUniform("depth_", depthUnit.getUnitNumber());
        shader_.setUniform("picking_", pickingUnit.getUnitNumber());

        utilgl::bindTextures(*image, colorUnit, depthUnit, pickingUnit);
        // Texture coordinates of the region we should copy
        vec2 texOffset = vec2(offset) / vec2(inputDim);
        vec2 texEnd = vec2(offset + dim) / vec2(inputDim);
        auto texCoordsBuffer = rect_.getTypedBuffer<buffertraits::TexcoordBuffer<2>>();
        auto coords = texCoordsBuffer->getEditableRAMRepresentation();
        coords->set(0, texOffset);
        coords->set(1, {texEnd.x, texOffset.y});
        coords->set(2, {texOffset.x, texEnd.y});
        coords->set(3, texEnd);

        utilgl::Enable<MeshGL> enable(rect_.getRepresentation<MeshGL>());
        utilgl::DepthFuncState depth(GL_ALWAYS);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        shader_.deactivate();
        utilgl::deactivateCurrentTarget();
        TextureUnit::setZeroUnit();
    }
}

void ImageSubsetGL::invokeEvent(Event* event) {
    // Allow panning but prevent interaction events from propagating because it does not
    // make sense to be able to pan and at the same time forward other events (e.g. zooom).
    // If you have a use for event propagation you should introduce a boolean property for
    // controlling if events should be handled by this processor. Alternatively, you can use
    // EventProperty, but make sure that it works on Trackpads since issues were spotted
    // when trying that route.
    switch (event->hash()) {
        case MouseEvent::chash():
            handleMousePan(static_cast<MouseEvent*>(event));
            event->markAsUsed();
            break;
        case TouchEvent::chash():
            handleTouchEvent(static_cast<TouchEvent*>(event));
            event->markAsUsed();
            break;
        case WheelEvent::chash():
            [[fallthrough]];
        case GestureEvent::chash():  // GestureEvents are better handles using TouchEvents
            event->markAsUsed();
        default:
            Processor::invokeEvent(event);
    }
}

void ImageSubsetGL::pan(ivec2 from, ivec2 to) {
    auto diff = from - to;
    auto clampDiff = [](auto diff, auto rangePressed, auto minRange, auto maxRange) {
        // Clamp to min/max range boundaries
        auto clampMovLeft = glm::clamp(diff + static_cast<int>(rangePressed.x),
                                       static_cast<int>(minRange), static_cast<int>(maxRange));
        auto clampMovRight = glm::clamp(diff + static_cast<int>(rangePressed.y),
                                        static_cast<int>(minRange), static_cast<int>(maxRange));
        // Prevent movement outside of boundaries
        auto diffLeft = clampMovLeft - static_cast<int>(rangePressed.x);
        auto diffRight = clampMovRight - static_cast<int>(rangePressed.y);
        if (std::abs(diffLeft) < std::abs(diffRight)) {
            return diffLeft;
        } else {
            return diffRight;
        }
    };
    auto xMov = clampDiff(diff.x, rangePressedX_, rangeX_.getRangeMin(), rangeX_.getRangeMax());
    auto yMov = clampDiff(diff.y, rangePressedY_, rangeY_.getRangeMin(), rangeY_.getRangeMax());
    {
        NetworkLock lock(this);
        rangeX_.set(size2_t(xMov + ivec2(rangePressedX_)));
        rangeY_.set(size2_t(yMov + ivec2(rangePressedY_)));
    }
}

void ImageSubsetGL::handleMousePan(MouseEvent* mouseEvent) {
    if (mouseEvent->button() == MouseButton::Left && mouseEvent->state() == MouseState::Press) {
        rangePressedX_ = rangeX_.get();
        rangePressedY_ = rangeY_.get();
        pressedPos_ = mouseEvent->pos();
    } else if (mouseEvent->buttonState() == MouseButton::Left &&
               mouseEvent->state() == MouseState::Move) {
        pan(pressedPos_, mouseEvent->pos());
    }

    mouseEvent->markAsUsed();
}

void ImageSubsetGL::handleTouchEvent(TouchEvent* touchEvent) {
    // Use the two closest points to extract translation, scaling and rotation
    const auto& touchPoints = touchEvent->touchPoints();
    if (touchPoints.empty()) return;

    TouchDevice::DeviceType type = touchEvent->getDevice() ? touchEvent->getDevice()->getType()
                                                           : TouchDevice::DeviceType::TouchScreen;
    bool panOp = false;
    // Use different approaches depending on device
    switch (type) {
        case TouchDevice::DeviceType::TouchScreen:
            // Pan using one touch point
            panOp = touchPoints.size() == 1;
            break;
        case TouchDevice::DeviceType::TouchPad:
            // Pan using two touch points similar to mouse
            // Stationary touch point is similar to mouse press
            panOp = touchPoints.size() > 1;  // && touchPoints[0].state() == TouchState::Stationary;
            break;
    }
    if (panOp) {
        // Assume that We require two touches when using trackpad since the first one will
        // occur when sweeping over the canvas
        size_t usePointIndex = (type == TouchDevice::DeviceType::TouchScreen) ? 0 : 1;
        const auto& point = touchPoints[usePointIndex];
        if (point.state() == TouchState::Started) {
            rangePressedX_ = rangeX_.get();
            rangePressedY_ = rangeY_.get();
            pressedPos_ = point.pos();
        }
        pan(point.pressedPos(), point.pos());

        touchEvent->markAsUsed();
    }
}

}  // namespace inviwo
