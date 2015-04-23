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

#include "imageoverlaygl.h"
#include <modules/opengl/glwrap/textureunit.h>
#include <inviwo/core/interaction/events/gestureevent.h>
#include <inviwo/core/interaction/events/touchevent.h>
#include <inviwo/core/interaction/interactionhandler.h>
#include <modules/opengl/textureutils.h>

namespace inviwo {

ProcessorClassIdentifier(ImageOverlayGL, "org.inviwo.ImageOverlayGL");
ProcessorDisplayName(ImageOverlayGL, "Image Overlay");
ProcessorTags(ImageOverlayGL, Tags::GL);
ProcessorCategory(ImageOverlayGL, "Image Operation");
ProcessorCodeState(ImageOverlayGL, CODE_STATE_EXPERIMENTAL);


OverlayProperty::OverlayProperty(std::string identifier,
                                  std::string displayName,
                                InvalidationLevel invalidationLevel,
                                PropertySemantics semantics)
    : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
    , size_("size", "Size", vec2(0.48f), vec2(0.0f), vec2(1.0f), vec2(0.01f))
    , pos_("position", "Position", vec2(0.25f), vec2(0.0f), vec2(1.0f), vec2(0.01f))
    , anchorPos_("anchor", "Anchor", vec2(0.0f), vec2(-1.0f), vec2(1.0f), vec2(0.01f))
    , blendMode_("blendMode", "Blending Mode")
{
    blendMode_.addOption("replace", "Replace", BlendMode::Replace);
    blendMode_.addOption("over", "Blend", BlendMode::Over);
    blendMode_.setSelectedValue(BlendMode::Over);
    blendMode_.setCurrentStateAsDefault();

    addProperty(size_);
    addProperty(pos_);
    addProperty(anchorPos_);
    addProperty(blendMode_);
}

void OverlayProperty::updateViewport(vec2 destDim) {
    vec2 pos(pos_.get());
    vec2 size(size_.get());
    vec2 anchor(anchorPos_.get());

    // consider anchor position
    vec2 shift = 0.5f * size * (anchorPos_.get() + vec2(1.0f, 1.0f));
    pos.x -= shift.x;
    // negate y axis
    pos.y = 1.0 - (pos.y + shift.y);

    pos *= destDim;
    size *= destDim;
    viewport_ = ivec4(pos.x, pos.y, size.x, size.y);
}


ImageOverlayGL::ImageOverlayGL()
    : Processor()
    , inport_("inport")
    , overlayPort_("overlay")
    , outport_("outport")
    , overlayInteraction_("overlayInteraction", "Overlay Interaction", false)
    , overlayProperty_("overlay", "Overlay")
    , shader_("img_texturequad.vert", "img_copy.frag")
    , overlayHandler_(this)
    , currentDim_(0u, 0u) 
{
    addPort(inport_);
    addPort(overlayPort_);
    overlayPort_.onChange(this, &ImageOverlayGL::overlayInportChanged);
    addPort(outport_);

    addProperty(overlayInteraction_);
    addProperty(overlayProperty_);

    overlayProperty_.onChange(this, &ImageOverlayGL::onStatusChange);

    addInteractionHandler(&overlayHandler_);
}

ImageOverlayGL::~ImageOverlayGL() {}

const std::vector<Inport*>& ImageOverlayGL::getInports(Event* e) const {
    currentInteractionInport_.clear();

    InteractionEvent* ie = dynamic_cast<InteractionEvent*>(e);
    if (ie) {
        if (overlayInteraction_.get()) {
            // Last clicked mouse position determines which inport is active
            // This is recorded with the interaction handler before-hand
            if (!viewCoords_.empty() && overlayPort_.isConnected()) {
                ivec2 pos = overlayHandler_.getActivePosition();
                ivec2 dim = outport_.getConstData()->getDimensions();
                pos.y = dim.y - pos.y;

                // single overlay
                if (inView(viewCoords_.front(), pos)) {
                    currentInteractionInport_.push_back(const_cast<ImageInport *>(&overlayPort_));
                }
                else {
                    // push main view
                    currentInteractionInport_.push_back(const_cast<ImageInport *>(&inport_));
                }
                /*
                // multiple overlay inputs
                std::vector<Inport*> inports = multiinport_.getInports();
                size_t minNum = std::min(inports.size(), viewCoords_.size());
                for (size_t i = 0; i < minNum; ++i) {
                if (inView(viewCoords_[i], pos)) {
                currentInteractionInport_.push_back(inports[i]);
                }
                }
                */
            }
            return currentInteractionInport_;
        }
        // interactions on overlays are disabled, forward event only to source imageport
        currentInteractionInport_.push_back(const_cast<ImageInport *>(&inport_));
        return currentInteractionInport_;
    }

    return Processor::getInports(e);
}

const std::vector<ivec4>& ImageOverlayGL::getViewCoords() const { return viewCoords_; }

bool ImageOverlayGL::isReady() const {
    return inport_.isReady();
}

void ImageOverlayGL::overlayInportChanged() {
    if (overlayPort_.isConnected()) {
        updateViewports(true);
    }

    updateDimensions();
}

void ImageOverlayGL::onStatusChange() {
    updateViewports(true);

    updateDimensions();
}

void ImageOverlayGL::updateDimensions() {
    uvec2 outDimU = outport_.getData()->getDimensions();
    vec2 outDim = vec2(outDimU.x, outDimU.y);
    if (overlayPort_.isConnected()) {
        uvec2 inDimU = overlayPort_.getDimensions();
        overlayPort_.setResizeScale(vec2(overlayProperty_.viewport_.z, overlayProperty_.viewport_.w) / outDim);
        uvec2 inDimNewU = uvec2(overlayProperty_.viewport_.z, overlayProperty_.viewport_.w);
        if (inDimNewU != inDimU && inDimNewU.x != 0 && inDimNewU.y != 0) {
            ResizeEvent e(inDimNewU);
            e.setPreviousSize(inDimU);
            overlayPort_.changeDataDimensions(&e);
        }
    }
}

void ImageOverlayGL::process() {
    ivec2 dim = outport_.getData()->getDimensions();

    TextureUnit colorUnit, depthUnit, pickingUnit;

    utilgl::activateAndClearTarget(outport_, COLOR_DEPTH_PICKING);

    shader_.activate();

    shader_.setUniform("color_", colorUnit.getUnitNumber());
    shader_.setUniform("depth_", depthUnit.getUnitNumber());
    shader_.setUniform("picking_", pickingUnit.getUnitNumber());

    // copy inport to outport before drawing the overlays
    utilgl::bindTextures(inport_.getData(), colorUnit.getEnum(), 
        depthUnit.getEnum(), pickingUnit.getEnum());
    utilgl::singleDrawImagePlaneRect();

    if (overlayPort_.hasData()) {
        // draw overlay
        switch (overlayProperty_.blendMode_.get()) {
        case OverlayProperty::BlendMode::Over:
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
        case OverlayProperty::BlendMode::Replace:
        default:
            glDisable(GL_BLEND);
            break;
        }

        glDepthFunc(GL_ALWAYS);

        utilgl::bindTextures(overlayPort_.getData(), colorUnit.getEnum(),
            depthUnit.getEnum(), pickingUnit.getEnum());
        ivec4 viewport = overlayProperty_.viewport_;
        glViewport(viewport.x, viewport.y, viewport.z, viewport.w);
        utilgl::singleDrawImagePlaneRect();

        glDisable(GL_BLEND);
        glDepthFunc(GL_LESS);
        // restore viewport
        glViewport(0, 0, dim.x, dim.y);
    }

    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

void ImageOverlayGL::updateViewports(bool force) {
    ivec2 dim(256, 256);
    if (outport_.isConnected()) dim = outport_.getData()->getDimensions();

    if (!force && (currentDim_ == dim)) return;  // no changes

    overlayProperty_.updateViewport(dim);
    viewCoords_.clear();
    if (overlayPort_.isConnected()) {
        viewCoords_.push_back(overlayProperty_.viewport_);
    }
    currentDim_ = dim;
}

ImageOverlayGL::ImageOverlayGLInteractionHandler::ImageOverlayGLInteractionHandler(ImageOverlayGL* src)
    : InteractionHandler()
    , src_(src)
    , activePositionChangeEvent_(ivec2(0), MouseEvent::MOUSE_BUTTON_LEFT,
                                 MouseEvent::MOUSE_STATE_PRESS, InteractionEvent::MODIFIER_NONE,
                                 uvec2(512))
    , viewportActive_(false)
    , activePosition_(ivec2(0)) {}

void ImageOverlayGL::ImageOverlayGLInteractionHandler::invokeEvent(Event* event) {

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

bool ImageOverlayGL::inView(const ivec4& view, const ivec2& pos) {
    return view.x < pos.x && pos.x < view.x + view.z && view.y < pos.y && pos.y < view.y + view.w;
}

}  // namespace
