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
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/touchevent.h>
#include <inviwo/core/network/networklock.h>

#include <modules/opengl/geometry/meshgl.h>
#include <modules/opengl/openglutils.h>
#include <modules/opengl/texture/textureutils.h>

namespace inviwo {

const ProcessorInfo ImageSubsetGL::processorInfo_{
    "org.inviwo.ImageSubsetGL",  // Class identifier
    "Image Subset",            // Display name
    "Image Operation",         // Category
    CodeState::Stable,          // Code state
    Tags::GL,                  // Tags
};
const ProcessorInfo ImageSubsetGL::getProcessorInfo() const { return processorInfo_; }

ImageSubsetGL::ImageSubsetGL()
    : Processor()
    , inport_("inputImage")
    , outport_("outputImage")
    , rangeX_("rangeX", "X Slices", 0, 256, 0, 256, 1, 1)
    , rangeY_("rangeY", "Y Slices", 0, 256, 0, 256, 1, 1)
    , mousePan_("trackballPan", "Pan",          [this](Event* e) { handlePan(e); }, std::make_unique<GeneralEventMatcher>([](Event* e) {
        auto isMouseEvent = e->hash() == MouseEvent::chash();
        return  isMouseEvent && static_cast<MouseEvent*>(e)->buttonState() == MouseButton::Left}))
    , touchGesture_("touchGesture", "Touch", [this](Event* e) { touchGesture(e); }, std::make_unique<GeneralEventMatcher>([](Event* e) { return e->hash() == TouchEvent::chash(); }))
    , shader_("img_texturequad.vert", "img_copy.frag") {
    addPort(inport_);
    addPort(outport_);
    addProperties(rangeX_, rangeY_, mousePan_, touchGesture_);
    dims_ = size2_t(1, 1);

    inport_.setOutportDeterminesSize(true);
    outport_.setHandleResizeEvents(false);
    // Since the ranges depend on the input image dimensions, we make sure to always
    // serialize them so we can do a proper renormalization when we load new data.
    rangeX_.setSerializationMode(PropertySerializationMode::All);
    rangeY_.setSerializationMode(PropertySerializationMode::All);

    inport_.onChange([this]() {
        NetworkLock lock(this);

        // Update to the new dimensions.
        dims_ = inport_.getData()->getDimensions();

        rangeX_.setRangeNormalized(size2_t(0, dims_.x));
        rangeY_.setRangeNormalized(size2_t(0, dims_.y));

        // set the new dimensions to default if we were to press reset
        rangeX_.setCurrentStateAsDefault();
        rangeY_.setCurrentStateAsDefault();
    });
    
    // Create a rectangle mesh for rendering.
    // Texture coordinates will be modified
    auto verticesBuffer =
    util::makeBuffer<vec2>({{-1.0f, -1.0f}, {1.0f, -1.0f}, {-1.0f, 1.0f}, {1.0f, 1.0f}});
    vec2 texOffset = vec2(0.f) / vec2(dims_);
    vec2 texEnd = vec2(1.f) / vec2(dims_);
    texCoordsBuffer_ =
    util::makeBuffer<vec2>({texOffset, {texEnd.x, texOffset.y}, {texOffset.x, texEnd.y}, texEnd});
    auto indices = util::makeIndexBuffer({0, 1, 2, 3});
    rect_.addBuffer(BufferType::PositionAttrib, verticesBuffer);
    rect_.addBuffer(BufferType::TexcoordAttrib, texCoordsBuffer_);
    rect_.addIndicies(Mesh::MeshInfo(DrawType::Triangles, ConnectivityType::Strip), indices);
}

ImageSubsetGL::~ImageSubsetGL() = default;

void ImageSubsetGL::process() {
    const ivec2 offset{rangeX_.get().x, rangeY_.get().x};
    const ivec2 dim = ivec2{rangeX_.get().y, rangeY_.get().y} - offset;
    if (dim == ivec2(dims_))
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
        vec2 texOffset = vec2(offset) / vec2(dims_);
        vec2 texEnd = vec2(offset+dim) / vec2(dims_);
        auto coords = texCoordsBuffer_->getEditableRAMRepresentation();
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
    
void ImageSubsetGL::pan(ivec2 diff) {
    auto getMovement = [](auto diff, auto rangePressed, auto minRange, auto maxRange) {
        // Clamp to min/max range boundaries
        auto clampMovLeft = glm::clamp(diff + static_cast<int>(rangePressed.x), static_cast<int>(minRange), static_cast<int>(maxRange));
        auto clampMovRight = glm::clamp(diff + static_cast<int>(rangePressed.y), static_cast<int>(minRange), static_cast<int>(maxRange));
        // Prevent movement outside of boundaries
        auto diffLeft = clampMovLeft-static_cast<int>(rangePressed.x);
        auto diffRight = clampMovRight-static_cast<int>(rangePressed.y);
        if (std::abs(diffLeft) < std::abs(diffRight)) {
            return diffLeft;
        } else {
            return diffRight;
        }
    };
    auto xMov = getMovement(diff.x, rangePressedX_, rangeX_.getRangeMin(), rangeX_.getRangeMax());
    auto yMov = getMovement(diff.y, rangePressedY_, rangeY_.getRangeMin(), rangeY_.getRangeMax());
    rangeX_.set(size2_t(xMov + ivec2(rangePressedX_)));
    rangeY_.set(size2_t(yMov + ivec2(rangePressedY_)));
}

void ImageSubsetGL::handlePan(Event* event) {
    auto mouseEvent = static_cast<MouseEvent*>(event);
    
    if (mouseEvent->state() == MouseState::Press && !mousePressed_) {
        rangePressedX_ = rangeX_.get();
        rangePressedY_ = rangeY_.get();
        lastInteractionPos_ = mouseEvent->pos();
        mousePressed_ = true;
    } else if (mouseEvent->state() == MouseState::Move) {
        auto diff = ivec2(mouseEvent->pos() - lastInteractionPos_);
        pan(diff);
    }
    if (mouseEvent->state() == MouseState::Release) {
        mousePressed_ = false;
    }
    
    event->markAsUsed();
}
    
void ImageSubsetGL::touchGesture(Event* event) {
    
    TouchEvent* touchEvent = static_cast<TouchEvent*>(event);
    
    // Use the two closest points to extract translation, scaling and rotation
    const auto& touchPoints = touchEvent->touchPoints();
    if (touchPoints.empty()) return;
    
    TouchDevice::DeviceType type = touchEvent->getDevice() ? touchEvent->getDevice()->getType()
    : TouchDevice::DeviceType::TouchScreen;
    bool panOp = false;
    bool panZoom = false;
    // Use different approaches depending on device
    switch (type) {
        case TouchDevice::DeviceType::TouchScreen:
            // Rotate when using one touch point
            panOp = touchPoints.size() == 1;
            // Pan/zoom when multiple touch points
            panZoom = touchPoints.size() > 1;
            break;
        case TouchDevice::DeviceType::TouchPad:
            // Pan using two touch points similar to mouse
            // Stationary touch point is similar to mouse press
            panOp = touchPoints.size() > 1;// && touchPoints[0].state() == TouchState::Stationary;
            // Is pinching/panning. Do not pan/zoom when panning
            panZoom = touchPoints.size() > 1 && !panOp;
        break;
    }
    if (panOp) {
        // Assume that We require two touches when using trackpad since the first one will occur
        // when sweeping over the canvas
        size_t usePointIndex = (type == TouchDevice::DeviceType::TouchScreen) ? 0 : 1;
        const auto& point = touchPoints[usePointIndex];
        if (point.state() == TouchState::Started) {
            rangePressedX_ = rangeX_.get();
            rangePressedY_ = rangeY_.get();
        }
        pan(point.pos() - point.pressedPos());
        

        // update positions
        lastInteractionPos_ = point.pos();
        event->markAsUsed();
    }
    if (panZoom) {
        const auto& touchPoint1 = touchPoints[0];
        const auto& touchPoint2 = touchPoints[1];
        
        const auto prevPos1 = touchPoint1.prevPosNormalized();
        const auto prevPos2 = touchPoint2.prevPosNormalized();
        const auto pos1 = touchPoint1.posNormalized();
        const auto pos2 = touchPoint2.posNormalized();
        
        auto v1(prevPos2 - prevPos1);
        auto v2(pos2 - pos1);
        
        touchEvent->markAsUsed();
    }
}

}  // namespace inviwo
