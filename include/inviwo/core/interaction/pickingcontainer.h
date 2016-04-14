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

class IVW_CORE_API PickingContainer {
public:
    PickingContainer();
    virtual ~PickingContainer();

    bool pickingEnabled();

    // Return true if picking was performed, otherwise false
    bool performMousePick(MouseEvent*);
    bool performTouchPick(TouchEvent*);

    void setPickingSource(std::shared_ptr<const Image> src);

protected:
    PickingObject* findPickingObject(const uvec2& coord);

    /** 
     * \brief normalized delta, i.e. (current - previous), with respect to canvas size
     *
     * @param previous  old position in screen coordinates
     * @param current   new position in screen coordinates
     * @return normalized delta
     */
    vec2 normalizedMovement(const uvec2& previous, const uvec2& current) const;

    /** 
     * \brief normalize coordinates with respect to the canvas size
     * 
     * @param coord   position in screen coordinates
     * @return normalized position
     */
    vec2 normalizedCoordinates(const uvec2& coord) const;

    /** 
     * \brief clamps 2D position to be within the given rectangle [0, dim - 1]
     * 
     * @param pos   coordinate in screen coordinates which needs to be clamped
     * @param dim   canvas dimensions used for clamping
     * @return clamped position
     */
    static uvec2 clampToScreenCoords(ivec2 mpos, ivec2 dim);

private:
    std::shared_ptr<const Image> src_;

    PickingObject* mousePickObj_;
    uvec2 prevMouseCoord_;
    bool mousePickingOngoing_;
    bool mouseIsDown_;

    bool touchPickingOn_;

    std::unordered_map<int, PickingObject*> touchPickObjs_;
    std::unordered_map<PickingObject*, std::vector<TouchPoint>> pickedTouchPoints_;
};

} // namespace

#endif // IVW_PICKINGCONTAINER_H