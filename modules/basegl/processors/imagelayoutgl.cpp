/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include "imagelayoutgl.h"
#include <modules/opengl/glwrap/textureunit.h>
#include <inviwo/core/interaction/events/gestureevent.h>
#include <inviwo/core/interaction/events/touchevent.h>
#include <modules/opengl/glwrap/shader.h>
#include <inviwo/core/interaction/interactionhandler.h>
#include <modules/opengl/textureutils.h>

namespace inviwo {

ProcessorClassIdentifier(ImageLayoutGL, "org.inviwo.ImageLayoutGL");
ProcessorDisplayName(ImageLayoutGL, "Image Layout");
ProcessorTags(ImageLayoutGL, Tags::GL);
ProcessorCategory(ImageLayoutGL, "Image Operation");
ProcessorCodeState(ImageLayoutGL, CODE_STATE_EXPERIMENTAL);

ImageLayoutGL::ImageLayoutGL()
    : Processor()
    , multiinport_("multiinport")
    , outport_("outport")
    , layout_("layout", "Layout")
    , horizontalSplitter_("horizontalSplitter", "Horizontal Split", 0.5f, 0.f, 1.f)
    , verticalSplitter_("verticalSplitter", "Vertical Split", 0.5f, 0.f, 1.f)
    , vertical3Left1RightSplitter_("vertical3Left1RightSplitter", "Split Position", 1.0f / 3.0f,
                                   0.f, 1.f)
    , vertical3Right1LeftSplitter_("vertical3Right1LeftSplitter", "Split Position", 2.0f / 3.0f,
                                   0.f, 1.f)
    , shader_(nullptr)
    , layoutHandler_(this)
    , currentLayout_(Layout::CrossSplit)
    , currentDim_(0u, 0u) {
    addPort(multiinport_);
    multiinport_.onChange(this, &ImageLayoutGL::multiInportChanged);
    addPort(outport_);
    layout_.addOption("single", "Single Only", Layout::Single);
    layout_.addOption("horizontalSplit", "Horizontal Split", Layout::HorizontalSplit);
    layout_.addOption("verticalSplit", "Vertical Split", Layout::VerticalSplit);
    layout_.addOption("crossSplit", "Cross Split", Layout::CrossSplit);
    layout_.addOption("threeRightOneLeftSplit", "Three Left, One Right", Layout::ThreeLeftOneRight);
    layout_.addOption("threeLeftOneRightSplit", "Three Right, One Left", Layout::ThreeRightOneLeft);
    layout_.setSelectedValue(Layout::CrossSplit);
    layout_.setCurrentStateAsDefault();

    addProperty(layout_);

    horizontalSplitter_.setVisible(false);
    horizontalSplitter_.onChange(this, &ImageLayoutGL::onStatusChange);
    addProperty(horizontalSplitter_);

    verticalSplitter_.setVisible(false);
    verticalSplitter_.onChange(this, &ImageLayoutGL::onStatusChange);
    addProperty(verticalSplitter_);

    vertical3Left1RightSplitter_.setVisible(false);
    vertical3Left1RightSplitter_.onChange(this, &ImageLayoutGL::onStatusChange);
    addProperty(vertical3Left1RightSplitter_);

    vertical3Right1LeftSplitter_.setVisible(false);
    vertical3Right1LeftSplitter_.onChange(this, &ImageLayoutGL::onStatusChange);
    addProperty(vertical3Right1LeftSplitter_);

    layout_.onChange(this, &ImageLayoutGL::onStatusChange);

    addInteractionHandler(&layoutHandler_);
}

ImageLayoutGL::~ImageLayoutGL() {}

void ImageLayoutGL::initialize() {
    Processor::initialize();
    shader_ = new Shader("img_texturequad.vert", "img_copy.frag");
    onStatusChange();
}

void ImageLayoutGL::deinitialize() {
    delete shader_;
    shader_ = nullptr;
    Processor::deinitialize();
}

const std::vector<Inport*>& ImageLayoutGL::getInports(Event* e) const {
    InteractionEvent* ie = dynamic_cast<InteractionEvent*>(e);
    // Last clicked mouse position determines which inport is active
    // This is recorded with the interactionhandler before-hand
    if (ie && !viewCoords_.empty()) {
        currentInteractionInport_.clear();
        if (multiinport_.isConnected()) {
            std::vector<Inport*> inports = multiinport_.getInports();
            size_t minNum = std::min(inports.size(), viewCoords_.size());
            ivec2 pos = layoutHandler_.getActivePosition();
            ivec2 dim = outport_.getConstData()->getDimensions();
            pos.y = dim.y - pos.y;

            for (size_t i = 0; i < minNum; ++i) {
                if (inView(viewCoords_[i], pos)) {
                    currentInteractionInport_.push_back(inports[i]);
                }
            }
        }
        return currentInteractionInport_;
    }
    return Processor::getInports(e);
}

const std::vector<ivec4>& ImageLayoutGL::getViewCoords() const { return viewCoords_; }

void ImageLayoutGL::multiInportChanged() {
    if (multiinport_.isConnected()) {
        updateViewports(true);
        std::vector<Inport*> inports = multiinport_.getInports();
        size_t minNum = std::min(inports.size(), viewCoords_.size());
        uvec2 outDimU = outport_.getData()->getDimensions();
        vec2 outDim = vec2(outDimU.x, outDimU.y);
        for (size_t i = 0; i < minNum; ++i) {
            ImageInport* imageInport = static_cast<ImageInport*>(inports[i]);
            imageInport->setResizeScale(vec2(viewCoords_[i].z, viewCoords_[i].w) / outDim);
            uvec2 inDimU = imageInport->getDimensions();
            if (inDimU == uvec2(8, 8)) {
                uvec2 inDimNewU = uvec2(viewCoords_[i].z, viewCoords_[i].w);
                ResizeEvent e(inDimNewU);
                e.setPreviousSize(inDimU);
                imageInport->changeDataDimensions(&e);
            }
        }
    }
}

void ImageLayoutGL::onStatusChange() {
    horizontalSplitter_.setVisible(false);
    verticalSplitter_.setVisible(false);
    vertical3Left1RightSplitter_.setVisible(false);
    vertical3Right1LeftSplitter_.setVisible(false);

    switch (layout_.getSelectedValue()) {
        case Layout::HorizontalSplit:
            horizontalSplitter_.setVisible(true);
            break;
        case Layout::VerticalSplit:
            verticalSplitter_.setVisible(true);
            break;
        case Layout::CrossSplit:
            horizontalSplitter_.setVisible(true);
            verticalSplitter_.setVisible(true);
            break;
        case Layout::ThreeLeftOneRight:
            vertical3Left1RightSplitter_.setVisible(true);
            break;
        case Layout::ThreeRightOneLeft:
            vertical3Right1LeftSplitter_.setVisible(true);
            break;
        case Layout::Single:
        default:
            break;
    }

    updateViewports(true);
    std::vector<Inport*> inports = multiinport_.getInports();
    size_t minNum = std::min(inports.size(), viewCoords_.size());
    uvec2 outDimU = outport_.getData()->getDimensions();
    vec2 outDim = vec2(outDimU.x, outDimU.y);
    for (size_t i = 0; i < minNum; ++i) {
        ImageInport* imageInport = static_cast<ImageInport*>(inports[i]);
        uvec2 inDimU = imageInport->getDimensions();
        imageInport->setResizeScale(vec2(viewCoords_[i].z, viewCoords_[i].w) / outDim);
        uvec2 inDimNewU = uvec2(viewCoords_[i].z, viewCoords_[i].w);
        if (inDimNewU != inDimU && inDimNewU.x != 0 && inDimNewU.y != 0) {
            ResizeEvent e(inDimNewU);
            e.setPreviousSize(inDimU);
            imageInport->changeDataDimensions(&e);
        }
    }
}

void ImageLayoutGL::process() {
    TextureUnit::setZeroUnit();
    std::vector<const Image*> images = multiinport_.getData();

    TextureUnit colorUnit, depthUnit, pickingUnit;

    utilgl::activateAndClearTarget(outport_, COLOR_DEPTH_PICKING);

    shader_->activate();
    shader_->setUniform("color_", colorUnit.getUnitNumber());
    shader_->setUniform("depth_", depthUnit.getUnitNumber());
    shader_->setUniform("picking_", pickingUnit.getUnitNumber());

    size_t minNum = std::min(images.size(), viewCoords_.size());
    for (size_t i = 0; i < minNum; ++i) {
        utilgl::bindTextures(images[i], colorUnit.getEnum(), depthUnit.getEnum(),
                             pickingUnit.getEnum());
        glViewport(viewCoords_[i].x, viewCoords_[i].y, viewCoords_[i].z, viewCoords_[i].w);
        utilgl::singleDrawImagePlaneRect();
    }

    ivec2 dim = outport_.getData()->getDimensions();
    glViewport(0, 0, dim.x, dim.y);

    shader_->deactivate();
    utilgl::deactivateCurrentTarget();
    TextureUnit::setZeroUnit();
}

void ImageLayoutGL::updateViewports(bool force) {
    ivec2 dim(256, 256);
    if (outport_.isConnected()) dim = outport_.getData()->getDimensions();

    if (!force && (currentDim_ == dim) && (currentLayout_ == layout_.get())) return;  // no changes

    viewCoords_.clear();
    int smallWindowDim = dim.y / 3;
    switch (layout_.getSelectedValue()) {
        case Layout::HorizontalSplit:

            // #########
            // #   1   #
            // #-------#
            // #   2   #
            // #########
            // X, Y, W, H

            viewCoords_.push_back(
                ivec4(0, horizontalSplitter_ * dim.y, dim.x, (1.f - horizontalSplitter_) * dim.y));
            viewCoords_.push_back(ivec4(0, 0, dim.x, horizontalSplitter_ * dim.y));
            break;
        case Layout::VerticalSplit:

            // #########
            // #   |   #
            // # 1 | 2 #
            // #   |   #
            // #########
            // X, Y, W, H

            viewCoords_.push_back(ivec4(0, 0, verticalSplitter_ * dim.x, dim.y));
            viewCoords_.push_back(
                ivec4(verticalSplitter_ * dim.x, 0, (1.0f - verticalSplitter_) * dim.x, dim.y));
            break;
        case Layout::CrossSplit:

            // #########
            // # 1 | 2 #
            // #-------#
            // # 3 | 4 #
            // #########
            // X, Y, W, H

            viewCoords_.push_back(ivec4(0, horizontalSplitter_ * dim.y, verticalSplitter_ * dim.x,
                                        (1.0f - horizontalSplitter_) * dim.y));

            viewCoords_.push_back(ivec4(verticalSplitter_ * dim.x, horizontalSplitter_ * dim.y,
                                        (1.0f - verticalSplitter_) * dim.x,
                                        (1.0f - horizontalSplitter_) * dim.y));

            viewCoords_.push_back(
                ivec4(0, 0, verticalSplitter_ * dim.x, horizontalSplitter_ * dim.y));

            viewCoords_.push_back(ivec4(verticalSplitter_ * dim.x, 0,
                                        (1.0f - verticalSplitter_) * dim.x,
                                        horizontalSplitter_ * dim.y));
            break;
        case Layout::ThreeLeftOneRight:

            // #############
            // # 1 |       #
            // #---|       #
            // # 2 |   4   #
            // #---|       #
            // # 3 |       #
            // #############
            // X, Y, W, H

            viewCoords_.push_back(
                ivec4(0, 2 * smallWindowDim, vertical3Left1RightSplitter_ * dim.x, smallWindowDim));
            viewCoords_.push_back(
                ivec4(0, smallWindowDim, vertical3Left1RightSplitter_ * dim.x, smallWindowDim));
            viewCoords_.push_back(
                ivec4(0, 0, vertical3Left1RightSplitter_ * dim.x, smallWindowDim));
            viewCoords_.push_back(ivec4(vertical3Left1RightSplitter_ * dim.x, 0,
                                        (1.f - vertical3Left1RightSplitter_) * dim.x, dim.y));
            break;
        case Layout::ThreeRightOneLeft:

            // #############
            // #       | 1 #
            // #       |---#
            // #   4   | 2 #
            // #       |---#
            // #       | 3 #
            // #############
            // X, Y, W, H

            viewCoords_.push_back(ivec4(vertical3Right1LeftSplitter_ * dim.x, 2 * smallWindowDim,
                                        (1.f - vertical3Right1LeftSplitter_) * dim.x,
                                        smallWindowDim));
            viewCoords_.push_back(ivec4(vertical3Right1LeftSplitter_ * dim.x, smallWindowDim,
                                        (1.f - vertical3Right1LeftSplitter_) * dim.x,
                                        smallWindowDim));
            viewCoords_.push_back(ivec4(vertical3Right1LeftSplitter_ * dim.x, 0,
                                        (1.f - vertical3Right1LeftSplitter_) * dim.x,
                                        smallWindowDim));
            viewCoords_.push_back(ivec4(0, 0, vertical3Right1LeftSplitter_ * dim.x, dim.y));
            break;
        case Layout::Single:
        default:
            viewCoords_.push_back(ivec4(0, 0, dim.x, dim.y));
    }

    currentDim_ = dim;
    currentLayout_ = layout_.get();
}

ImageLayoutGL::ImageLayoutGLInteractionHandler::ImageLayoutGLInteractionHandler(ImageLayoutGL* src)
    : InteractionHandler()
    , src_(src)
    , activePositionChangeEvent_(ivec2(0), MouseEvent::MOUSE_BUTTON_LEFT,
                                 MouseEvent::MOUSE_STATE_PRESS, InteractionEvent::MODIFIER_NONE,
                                 uvec2(512))
    , viewportActive_(false)
    , activePosition_(ivec2(0)) {}

void ImageLayoutGL::ImageLayoutGLInteractionHandler::invokeEvent(Event* event) {
    const std::vector<ivec4>& viewCoords = src_->getViewCoords();

    MouseEvent* mouseEvent = dynamic_cast<MouseEvent*>(event);
    if (mouseEvent) {
        if (!viewportActive_ && mouseEvent->state() == activePositionChangeEvent_.state()) {
            viewportActive_ = true;
            activePosition_ = mouseEvent->pos();
        } else if (viewportActive_ && mouseEvent->state() == MouseEvent::MOUSE_STATE_RELEASE) {
            viewportActive_ = false;
        }

        ivec2 mPos = mouseEvent->pos();
        ivec2 cSize = mouseEvent->canvasSize();
        // Flip y-coordinate to bottom->up
        ivec2 activePosition(activePosition_.x, cSize.y - activePosition_.y);
        for (size_t i = 0; i < viewCoords.size(); ++i) {
            if (inView(viewCoords[i], activePosition)) {
                ivec2 vc = ivec2(viewCoords[i].x, cSize.y - viewCoords[i].y - viewCoords[i].w);
                mouseEvent->modify(mPos - vc, uvec2(viewCoords[i].z, viewCoords[i].w));
                break;
            }
        }

        return;
    }

    GestureEvent* gestureEvent = dynamic_cast<GestureEvent*>(event);
    if (gestureEvent) {
        vec2 mPosNorm = gestureEvent->screenPosNormalized();
        vec2 cSize = gestureEvent->canvasSize();
        vec2 mPos = mPosNorm * cSize;
        vec2 activePosition(mPos.x, cSize.y - mPos.y);
        for (size_t i = 0; i < viewCoords.size(); ++i) {
            if (inView(viewCoords[i], activePosition)) {
                vec2 vc = vec2(viewCoords[i].x, cSize.y - viewCoords[i].y - viewCoords[i].w);
                gestureEvent->modify((mPos - vc) / vec2(viewCoords[i].zw()));
                break;
            }
        }

        return;
    }

    TouchEvent* touchEvent = dynamic_cast<TouchEvent*>(event);
    if (touchEvent) {
        if (!viewportActive_ && touchEvent->state() == TouchEvent::TOUCH_STATE_STARTED) {
            viewportActive_ = true;
            activePosition_ = touchEvent->pos();
        } else if (viewportActive_ && touchEvent->state() == TouchEvent::TOUCH_STATE_ENDED) {
            viewportActive_ = false;
        }
        return;
    }
}

bool ImageLayoutGL::inView(const ivec4& view, const ivec2& pos) {
    return view.x < pos.x && pos.x < view.x + view.z && view.y < pos.y && pos.y < view.y + view.w;
}

}  // namespace
