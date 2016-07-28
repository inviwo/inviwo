/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2016 Inviwo Foundation
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

#ifndef IVW_PICKINGCONTAINER_H
#define IVW_PICKINGCONTAINER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/interaction/events/touchevent.h>
#include <unordered_map>

namespace inviwo {

class Image;
class PickingObject;
class MouseEvent;
class WheelEvent;

class IVW_CORE_API PickingContainer {
public:
    PickingContainer();
    virtual ~PickingContainer();

    bool pickingEnabled();

    void propagateEvent(Event*);

    void setPickingSource(std::shared_ptr<const Image> src);

protected:
    void performMousePick(MouseEvent*);
    void performWheelPick(WheelEvent*);
    void performTouchPick(TouchEvent*);

    PickingObject* findPickingObject(const uvec2& coord);

    /** 
     * \brief clamps 2D position to be within the given rectangle [0, dim - 1]
     * 
     * @param mpos coordinate in screen coordinates which needs to be clamped
     * @param dim canvas dimensions used for clamping
     * @return clamped position
     */
    static uvec2 clampToScreenCoords(dvec2 mpos, ivec2 dim);

private:
    std::shared_ptr<const Image> src_;

    bool mousePressed_ = false;
    PickingObject* lastPickObj_ = nullptr;
    PickingObject* pressedPickObj_ = nullptr;

    bool touchPickingOn_;

    std::unordered_map<int, PickingObject*> touchPickObjs_;
    std::unordered_map<PickingObject*, std::vector<TouchPoint>> pickedTouchPoints_;
};

} // namespace

#endif // IVW_PICKINGCONTAINER_H