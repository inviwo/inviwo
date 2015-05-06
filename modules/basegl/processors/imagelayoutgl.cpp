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
    , shader_("img_texturequad.vert", "img_copy.frag")
    , viewManager_()
    , currentLayout_(Layout::CrossSplit)
    , currentDim_(0u, 0u) {
    
    addPort(multiinport_);
    
    multiinport_.onConnect([this](){
        ResizeEvent e(currentDim_);
        propagateResizeEvent(&e, &outport_);
    });
    
    multiinport_.onDisconnect([this](){
        ResizeEvent e(currentDim_);
        propagateResizeEvent(&e, &outport_);
    });
    
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
}

ImageLayoutGL::~ImageLayoutGL() {}

void ImageLayoutGL::initialize() {
    Processor::initialize();
    onStatusChange();
}


void ImageLayoutGL::propagateEvent(Event* event) {
    invokeEvent(event);
    
    viewManager_.registerEvent(event);

    size_t activeView = viewManager_.getActiveView();
    auto data = multiinport_.getConnectedOutports();

    if (activeView < data.size() ) {
        Event* newEvent = viewManager_.newAdjustedEvent(event);
        multiinport_.propagateEvent(newEvent, data[activeView]);
        if (newEvent->hasBeenUsed()) event->markAsUsed();
        delete newEvent;
    }
}

const std::vector<ivec4>& ImageLayoutGL::getViewCoords() const { return viewManager_.getViews(); }

bool ImageLayoutGL::propagateResizeEvent(ResizeEvent* resizeEvent, Outport* source) {
    updateViewports(resizeEvent->size(), true);
    auto outports = multiinport_.getConnectedOutports();
    size_t minNum = std::min(outports.size(), viewManager_.size());

    for (size_t i = 0; i < minNum; ++i) {
        ResizeEvent e(uvec2(viewManager_[i].z, viewManager_[i].w));
        multiinport_.propagateResizeEvent(&e, static_cast<ImageOutport*>(outports[i]));
    }

    return false;
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

    ResizeEvent e(currentDim_);
    propagateResizeEvent(&e, &outport_);
}

void ImageLayoutGL::process() {
    TextureUnit::setZeroUnit();
    std::vector<const Image*> images = multiinport_.getVectorData();

    TextureUnit colorUnit, depthUnit, pickingUnit;
    utilgl::activateAndClearTarget(outport_, COLOR_DEPTH_PICKING);

    shader_.activate();
    shader_.setUniform("color_", colorUnit.getUnitNumber());
    shader_.setUniform("depth_", depthUnit.getUnitNumber());
    shader_.setUniform("picking_", pickingUnit.getUnitNumber());

    size_t minNum = std::min(images.size(), viewManager_.size());
    for (size_t i = 0; i < minNum; ++i) {
        utilgl::bindTextures(images[i], colorUnit, depthUnit, pickingUnit);
        glViewport(viewManager_[i].x, viewManager_[i].y, viewManager_[i].z, viewManager_[i].w);
        utilgl::singleDrawImagePlaneRect();
    }

    ivec2 dim = outport_.getData()->getDimensions();
    glViewport(0, 0, dim.x, dim.y);

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
    TextureUnit::setZeroUnit();
}

void ImageLayoutGL::updateViewports(ivec2 dim, bool force) {
    if (!force && (currentDim_ == dim) && (currentLayout_ == layout_.get())) return;  // no changes

    viewManager_.clear();
    int smallWindowDim = dim.y / 3;
    switch (layout_.getSelectedValue()) {
        case Layout::HorizontalSplit:

            // #########
            // #   1   #
            // #-------#
            // #   2   #
            // #########
            // X, Y, W, H

            viewManager_.push_back(
                ivec4(0, horizontalSplitter_ * dim.y, dim.x, (1.f - horizontalSplitter_) * dim.y));
            viewManager_.push_back(ivec4(0, 0, dim.x, horizontalSplitter_ * dim.y));
            break;
        case Layout::VerticalSplit:

            // #########
            // #   |   #
            // # 1 | 2 #
            // #   |   #
            // #########
            // X, Y, W, H

            viewManager_.push_back(ivec4(0, 0, verticalSplitter_ * dim.x, dim.y));
            viewManager_.push_back(
                ivec4(verticalSplitter_ * dim.x, 0, (1.0f - verticalSplitter_) * dim.x, dim.y));
            break;
        case Layout::CrossSplit:

            // #########
            // # 1 | 2 #
            // #-------#
            // # 3 | 4 #
            // #########
            // X, Y, W, H

            viewManager_.push_back(ivec4(0, horizontalSplitter_ * dim.y, verticalSplitter_ * dim.x,
                                        (1.0f - horizontalSplitter_) * dim.y));

            viewManager_.push_back(ivec4(verticalSplitter_ * dim.x, horizontalSplitter_ * dim.y,
                                        (1.0f - verticalSplitter_) * dim.x,
                                        (1.0f - horizontalSplitter_) * dim.y));

            viewManager_.push_back(
                ivec4(0, 0, verticalSplitter_ * dim.x, horizontalSplitter_ * dim.y));

            viewManager_.push_back(ivec4(verticalSplitter_ * dim.x, 0,
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

            viewManager_.push_back(
                ivec4(0, 2 * smallWindowDim, vertical3Left1RightSplitter_ * dim.x, smallWindowDim));
            viewManager_.push_back(
                ivec4(0, smallWindowDim, vertical3Left1RightSplitter_ * dim.x, smallWindowDim));
            viewManager_.push_back(
                ivec4(0, 0, vertical3Left1RightSplitter_ * dim.x, smallWindowDim));
            viewManager_.push_back(ivec4(vertical3Left1RightSplitter_ * dim.x, 0,
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

            viewManager_.push_back(ivec4(vertical3Right1LeftSplitter_ * dim.x, 2 * smallWindowDim,
                                        (1.f - vertical3Right1LeftSplitter_) * dim.x,
                                        smallWindowDim));
            viewManager_.push_back(ivec4(vertical3Right1LeftSplitter_ * dim.x, smallWindowDim,
                                        (1.f - vertical3Right1LeftSplitter_) * dim.x,
                                        smallWindowDim));
            viewManager_.push_back(ivec4(vertical3Right1LeftSplitter_ * dim.x, 0,
                                        (1.f - vertical3Right1LeftSplitter_) * dim.x,
                                        smallWindowDim));
            viewManager_.push_back(ivec4(0, 0, vertical3Right1LeftSplitter_ * dim.x, dim.y));
            break;
        case Layout::Single:
        default:
            viewManager_.push_back(ivec4(0, 0, dim.x, dim.y));
    }

    currentDim_ = dim;
    currentLayout_ = layout_.get();
}

ImageLayoutGL::ViewManager::ViewManager()
    : viewportActive_(false)
    , activePosition_(ivec2(0))
    , activeView_(0) {}

void ImageLayoutGL::ViewManager::registerEvent(Event* event) {
    if (MouseEvent* mouseEvent = dynamic_cast<MouseEvent*>(event)) {
        activePosition_ = flipY(mouseEvent->pos(), mouseEvent->canvasSize());
        if (!viewportActive_ && mouseEvent->state() == MouseEvent::MOUSE_STATE_PRESS) {
            viewportActive_ = true;
            activeView_ = findView(activePosition_);
        } else if (viewportActive_ && mouseEvent->state() == MouseEvent::MOUSE_STATE_RELEASE) {
            viewportActive_ = false;
        }

    } else if (GestureEvent* gestureEvent = dynamic_cast<GestureEvent*>(event)) {
        activePosition_ = flipY(gestureEvent->canvasSize() * gestureEvent->screenPosNormalized(),
                                gestureEvent->canvasSize());
        if (!viewportActive_ && gestureEvent->state() == GestureEvent::GESTURE_STATE_STARTED) {
            viewportActive_ = true;
            activeView_ = findView(activePosition_);
        } else if (viewportActive_ && gestureEvent->state() == GestureEvent::GESTURE_STATE_ENDED) {
            viewportActive_ = false;
        }

    } else if (TouchEvent* touchEvent = dynamic_cast<TouchEvent*>(event)) {
        // TODO fix TouchEvents
        //         activePosition_ = flipY(touchEvent->pos(), touchEvent->canvasSize());
        //         if (!viewportActive_ && touchEvent->state() == TouchEvent::TOUCH_STATE_STARTED) {
        //             viewportActive_ = true;
        //             activeView_ = findView(activePosition_);
        //         } else if (viewportActive_ && touchEvent->state() ==
        //         TouchEvent::TOUCH_STATE_ENDED) {
        //             viewportActive_ = false;
        //         }
    }
}

Event* ImageLayoutGL::ViewManager::newAdjustedEvent(Event* oldEvent) {
    if (MouseEvent* mouseEvent = dynamic_cast<MouseEvent*>(oldEvent)) {
        MouseEvent* newEvent = mouseEvent->clone();
        const ivec4& view = views_[activeView_];
        newEvent->modify(flipY(activePosition_ - ivec2(view.x, view.y), ivec2(view.z, view.w)),
                         uvec2(view.z, view.w));
        return newEvent;

    } else if (GestureEvent* gestureEvent = dynamic_cast<GestureEvent*>(oldEvent)) {
        GestureEvent* newEvent = gestureEvent->clone();
        const ivec4& view = views_[activeView_];
        newEvent->modify(
            vec2(flipY(activePosition_ - ivec2(view.x, view.y), ivec2(view.z, view.w))) /
            vec2(view.z, view.w));
        return newEvent;

    } else if (TouchEvent* touchEvent = dynamic_cast<TouchEvent*>(oldEvent)) {
        // TODO fix TouchEvents
        TouchEvent* newEvent = touchEvent->clone();
        const ivec4& view = views_[activeView_];
        //  newEvent->modify(flipY(activePosition_ - ivec2(view.x, view.y),ivec2(view.z, view.w)),
        //  uvec2(view.z, view.w));

        return newEvent;
    }
}

const std::vector<ivec4>& ImageLayoutGL::ViewManager::getViews() const {
    return views_;
}

void ImageLayoutGL::ViewManager::push_back(ivec4 view) {
    views_.push_back(view);
}

ivec4& ImageLayoutGL::ViewManager::operator[](size_t ind) {
    return views_[ind];
}

size_t ImageLayoutGL::ViewManager::size() const {
    return views_.size();
}

void ImageLayoutGL::ViewManager::clear() {
    views_.clear();
}

size_t ImageLayoutGL::ViewManager::findView(ivec2 pos) const {
    for (size_t i = 0; i < views_.size(); ++i) {
        if (inView(views_[i], pos)) {
            return i;
        }
    }
    return 0;
}

inviwo::ivec2 ImageLayoutGL::ViewManager::flipY(ivec2 pos, ivec2 size) {
    return ivec2(pos.x, size.y - pos.y);
}

bool ImageLayoutGL::ViewManager::inView(const ivec4& view, const ivec2& pos) {
    return view.x < pos.x && pos.x < view.x + view.z && view.y < pos.y && pos.y < view.y + view.w;
}

}  // namespace
