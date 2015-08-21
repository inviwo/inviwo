/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#ifndef IVW_VIEWMANAGER_H
#define IVW_VIEWMANAGER_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/interaction/events/event.h>

namespace inviwo {

/**
 * \class ViewManager
 *
 * \brief A viewport manager for layout processors like imagelayout.
 * Viewports are added using the following coordinate system:
 *
 * y ^
 *   |
 *   |
 *   ------> x
 * Example for a (512,512) viewport split along the horizontal axis:
 *   _________(512,512)
 *   |   2   |
 *   |_______|(512,256)
 *   |   1   |
 *   |_______|
 *              x  y    width height
 * Viewport 1: (0, 0,   512,  256)
 * Viewport 2: (0, 256, 512,  256)
 */
class IVW_MODULE_BASEGL_API ViewManager {
public:
    ViewManager();

    /** 
     * \brief Creates a new event for the currently active viewport. 
     * Sets the viewport that the event lie in as active when starting an action (pressing/touching).
     * Further events will be propagated to the active viewport until a release appears.
     * Returns null if the initiating (pressing/touching) event is outside the added viewports.
     *
     * @param Event * event The event to transform.
     * @return Event* An event transformed to the active viewport or null if initiated outside of added views.
     */
    Event* registerEvent(const Event* event);
    /** 
     * \brief Get the untransformed position, relative to the original viewport size, in the coordinate system of the ViewManager.
     * 
     * @return ivec2 The pixel position in the same coordinate system as the ViewManager.
     */
    ivec2 getActivePosition() const { return activePosition_; }
    /** 
     * \brief Returns the index of the last active view. Returns -1 if no active view has been set.
     * 
     * @return int Index of active view, -1 if not set.
     */
    int getActiveView() const { return activeView_; }

    const std::vector<ivec4>& getViews() const;
    /** 
     * \brief Add a viewport (x,y width,height) using the following coordinate system:
     * y ^
     *   |
     *   |
     *   ------> x
     *
     * @see ViewManager
     * @param ivec4 view Position and size of viewport (x,y width,height)
     */
    void push_back(ivec4 view);

    /** 
     * \brief Return viewport using index ind.
     *
     * @param size_t ind Viewport index [0 size()-1]
     * @return ivec4& 
     */
    ivec4& operator[](size_t ind);
    size_t size() const;
    void clear();

private:
    int findView(ivec2 pos) const;
    static ivec2 flipY(ivec2 pos, ivec2 size);
    static bool inView(const ivec4& view, const ivec2& pos);

    bool viewportActive_;
    ivec2 activePosition_;
    int activeView_;
    std::vector<ivec4> views_;
};

}  // namespace

#endif  // IVW_VIEWMANAGER_H
