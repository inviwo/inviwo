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

namespace inviwo {

PickingContainer::PickingContainer()
    : src_(nullptr)
    , mousePickObj_(nullptr)
    , prevMouseCoord_(uvec2(0, 0))
    , mousePickingOngoing_(false)
    , mouseIsDown_(false)
    , touchPickingOn_(false)
{}

PickingContainer::~PickingContainer() {}

bool PickingContainer::pickingEnabled() {
    return PickingManager::getPtr()->pickingEnabled();
}

bool PickingContainer::performMousePick(MouseEvent* e) {
    if (!pickingEnabled() || e->button() == MouseEvent::MOUSE_BUTTON_NONE)
        return false;

    if (touchPickingOn_)
        return true;

    if (e->state() == MouseEvent::MOUSE_STATE_RELEASE){
        mouseIsDown_ = false;
        mousePickingOngoing_ = false;
        return false;
    }
    else if (!mouseIsDown_ || e->state() == MouseEvent::MOUSE_STATE_PRESS){
        mouseIsDown_ = true;

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

    return false;
}

bool PickingContainer::performTouchPick(TouchEvent* e) {
    if (!pickingEnabled())
        return false;

    std::vector<TouchPoint>& touchPoints = e->getTouchPoints();

    // Clear the picked touch point map
    pickedTouchPoints_.clear();

    if (touchPoints.size() > 1 || touchPoints[0].state() != TouchPoint::TOUCH_STATE_ENDED)
        touchPickingOn_ = true;
    else
        touchPickingOn_ = false;

    std::unordered_map<int, PickingObject*>::iterator touchPickObjs_it;
    std::unordered_map<PickingObject*, std::vector<TouchPoint>>::iterator pickedTouchPoints_it;

    auto touchPoint = touchPoints.begin();
    while (touchPoint != touchPoints.end()) {
        bool isAssociated = false;
        if (touchPoint->state() == TouchPoint::TOUCH_STATE_STARTED) {
            // Find out if new touch point is touching inside a picking object
            uvec2 coord = mousePosToPixelCoordinates(touchPoint->getPos(), e->canvasSize());
            PickingObject* pickObj = findPickingObject(coord);

            // If it is, put it in the TouchIDPickingMap
            if (pickObj) {
                touchPickObjs_.insert(std::pair<int, PickingObject*>(touchPoint->getId(), pickObj));

                // Associate touch point with picking object
                // which can already have other associated touch points.
                pickedTouchPoints_it = pickedTouchPoints_.find(pickObj);
                if (pickedTouchPoints_it != pickedTouchPoints_.end()){
                    pickedTouchPoints_it->second.push_back(*touchPoint);
                }
                else{
                    pickedTouchPoints_.insert(std::pair<PickingObject*, 
                        std::vector<TouchPoint>>(pickObj, std::vector<TouchPoint>{*touchPoint}));
                }
                isAssociated = true;
            }
        }
        else if (touchPoint->state() == TouchPoint::TOUCH_STATE_ENDED) {
            // Erase touch point from TouchIDPickingMap
            size_t numberOfErasedElements = touchPickObjs_.erase(touchPoint->getId());
            isAssociated = (numberOfErasedElements > 0);
        }
        else {
            // Find out if touch point is in the TouchIDPickingMap
            // If it exists, associate touch point with picking object
            touchPickObjs_it = touchPickObjs_.find(touchPoint->getId());
            if (touchPickObjs_it != touchPickObjs_.end()){
                // Associate touch point with picking object
                // which can already have other associated touch points.
                pickedTouchPoints_it = pickedTouchPoints_.find(touchPickObjs_it->second);
                if (pickedTouchPoints_it != pickedTouchPoints_.end()){
                    pickedTouchPoints_it->second.push_back(*touchPoint);
                }
                else{
                    pickedTouchPoints_.insert(std::pair<PickingObject*, 
                        std::vector<TouchPoint>>(touchPickObjs_it->second, std::vector<TouchPoint>{*touchPoint}));
                }
                isAssociated = true;
            }
        }
        // Removed touch point from the actual event if it was associated with a picking object
        if (isAssociated)
            touchPoint = touchPoints.erase(touchPoint);
        else
            ++touchPoint;
    }

    // Build touch event for all picking objects with associated touch points
    for (pickedTouchPoints_it = pickedTouchPoints_.begin(); pickedTouchPoints_it != pickedTouchPoints_.end(); ++pickedTouchPoints_it){
        // Treat one touch point the same as mouse event, for now
        if (pickedTouchPoints_it->second.size() == 1){
            uvec2 coord = mousePosToPixelCoordinates(pickedTouchPoints_it->second[0].getPos(), e->canvasSize());
            if (pickedTouchPoints_it->second[0].state() & TouchPoint::TOUCH_STATE_STARTED){
                pickedTouchPoints_it->first->setPickingPosition(normalizedCoordinates(coord));
                pickedTouchPoints_it->first->setPickingDepth(pickedTouchPoints_it->second[0].getDepth());
                pickedTouchPoints_it->first->setPickingMove(vec2(0.f, 0.f));
            }
            else{
                uvec2 prevCoord = mousePosToPixelCoordinates(pickedTouchPoints_it->second[0].getPrevPos(), e->canvasSize());
                pickedTouchPoints_it->first->setPickingMove(pixelMoveVector(prevCoord, coord));
            }
            // One touch point is currently treated as mouse event as well...
            // So prepare for that
            prevMouseCoord_ = coord;
            mousePickObj_ = pickedTouchPoints_it->first;
            mousePickingOngoing_ = true;
        }

        pickedTouchPoints_it->first->setPickingTouchEvent(TouchEvent(pickedTouchPoints_it->second, e->canvasSize()));
    }

    // One touch point is currently treated as mouse event as well...
    // So prepare for that
    if (touchPoints.size() == 1){
        prevMouseCoord_ = mousePosToPixelCoordinates(touchPoints[0].getPos(), e->canvasSize());
        touchPickingOn_ = false;
    }

    // Mark all picking objects in TouchIDPickingMap as picked.
    for (touchPickObjs_it = touchPickObjs_.begin(); touchPickObjs_it != touchPickObjs_.end(); ++touchPickObjs_it)
        touchPickObjs_it->second->picked();

    return !touchPickObjs_.empty();
}

void PickingContainer::setPickingSource(const Image* src) {
    src_ = src;
}

PickingObject* PickingContainer::findPickingObject(const uvec2& coord){
    if (pickingEnabled() && src_) {
        const Layer* pickingLayer = src_->getPickingLayer();

        if (pickingLayer) {
            const LayerRAM* pickingLayerRAM = pickingLayer->getRepresentation<LayerRAM>();
            dvec4 value = pickingLayerRAM->getValueAsVec4Double(coord);
            dvec3 pickedColor = (value.a > 0.0 ? value.rgb() : dvec3(0.0));
            uvec3 color(pickedColor*255.0);
            return PickingManager::getPtr()->getPickingObjectFromColor(color);
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