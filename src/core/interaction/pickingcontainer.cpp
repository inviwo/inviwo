/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include <inviwo/core/interaction/pickingcontainer.h>
#include <inviwo/core/interaction/pickingmanager.h>
#include <inviwo/core/datastructures/image/image.h>
#include <inviwo/core/datastructures/image/layerram.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/touchevent.h>

namespace inviwo {

PickingContainer::PickingContainer()
    : src_(nullptr), mousePickObj_(nullptr), prevMouseCoord_(uvec2(0, 0)), mousePickingOngoing_(false){};

PickingContainer::~PickingContainer() {};

bool PickingContainer::performMousePick(MouseEvent* e) {
    if (e->button() == MouseEvent::MOUSE_BUTTON_LEFT && e->state() == MouseEvent::MOUSE_STATE_PRESS){
        uvec2 coord = mousePosToPixelCoordinates(e->pos(), e->canvasSize());
        prevMouseCoord_ = coord;

        mousePickObj_ = findPickingObject(coord);

        if (mousePickObj_) {
            mousePickingOngoing_ = true;
            mousePickObj_->setPickingPosition(normalizedCoordinates(coord));
            mousePickObj_->setPickingDepth(e->depth());
            mousePickObj_->setPickingMouseEvent(*e);

            mousePickObj_->setPickingMove(vec2(0.f, 0.f));
            mousePickObj_->picked();
            return true;
        }
        else{
            mousePickingOngoing_ = false;
            return false;
        }
    }
    else if (e->state() == MouseEvent::MOUSE_STATE_MOVE){
        if (mousePickingOngoing_){
            uvec2 coord = mousePosToPixelCoordinates(e->pos(), e->canvasSize());
            mousePickObj_->setPickingMove(pixelMoveVector(prevMouseCoord_, coord));
            mousePickObj_->setPickingMouseEvent(*e);
            prevMouseCoord_ = coord;
            mousePickObj_->picked();
            return true;
        }
        else
            return false;
    }
    else if (e->button() == MouseEvent::MOUSE_BUTTON_LEFT && e->state() == MouseEvent::MOUSE_STATE_RELEASE){
        mousePickingOngoing_ = false;
        return false;
    }

    return false;
}

bool PickingContainer::performTouchPick(TouchEvent*) {
    //TODO: Implement
    return false;
}

void PickingContainer::setPickingSource(const Image* src) {
    src_ = src;
}

PickingObject* PickingContainer::findPickingObject(const uvec2& coord){
    if (PickingManager::getPtr()->pickingEnabled() && src_) {
        const Layer* pickingLayer = src_->getPickingLayer();

        if (pickingLayer) {
            const LayerRAM* pickingLayerRAM = pickingLayer->getRepresentation<LayerRAM>();
            dvec4 value = pickingLayerRAM->getValueAsVec4Double(coord);
            dvec3 pickedColor = (value.a > 0.0 ? value.rgb() : dvec3(0.0));
            DataVec3UINT8::type pickedColorUINT8;
            DataVec3UINT8::get()->vec3DoubleToValue(pickedColor*255.0, &pickedColorUINT8);
            return PickingManager::getPtr()->getPickingObjectFromColor(pickedColorUINT8);
        }
    }

    return nullptr;
}

vec2 PickingContainer::pixelMoveVector(const uvec2& previous, const uvec2& current) {
    return vec2((static_cast<float>(current.x)-static_cast<float>(previous.x))/static_cast<float>(src_->getDimensions().x),
                (static_cast<float>(current.y)-static_cast<float>(previous.y))/static_cast<float>(src_->getDimensions().y));
}

vec2 PickingContainer::normalizedCoordinates(const uvec2& coord) {
    return vec2(static_cast<float>(coord.x)/static_cast<float>(src_->getDimensions().x),
                static_cast<float>(coord.y)/static_cast<float>(src_->getDimensions().y));
}

uvec2 PickingContainer::mousePosToPixelCoordinates(ivec2 mpos, ivec2 dim) {
    ivec2 pos = mpos;
    pos.x = std::max(pos.x - 1, 0);
    pos.x = std::min(pos.x, dim.x - 1);

    pos.y = std::max(dim.y - pos.y - 1, 0);
    pos.y = std::min(pos.y, dim.y - 1);

    return uvec2(pos);
}

} // namespace