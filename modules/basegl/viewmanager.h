/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2016 Inviwo Foundation
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
    struct View {
        View(const ivec2& p, const ivec2& s) : pos(p), size(s) {}; 
        View(const ivec4& m) : pos(m.xy()), size(m.zw()) {};

        ivec2 pos;
        ivec2 size;
    };
    using ViewList = std::vector<View>;
    using ViewId = size_t;

    ViewManager();

    /**
     * \brief Creates a new event for the currently active viewport.
     * Sets the viewport that the event lie in as active when starting an action
     * (pressing/touching).
     * Further events will be propagated to the active viewport until a release appears.
     * Returns null if the initiating (pressing/touching) event is outside the added viewports.
     *
     * @param event The event to transform.
     * @return An event transformed to the active viewport or null if initiated outside of
     * added views.
     */
    std::unique_ptr<Event> registerEvent(const Event* event);

    /**
     * \brief Returns a pair with a bool of whether a view was found, and the index of the found
     * view.
     */
    std::pair<bool, ViewId> getSelectedView() const;

    const ViewList& getViews() const;

    /** 
     * \brief Add a viewport (x,y width,height) using the following coordinate system:
     * y ^
     *   |
     *   |
     *   ------> x
     *
     * @see ViewManager
     */
    void push_back(View view);

    /**
    * \brief Erase a previously defined viewport (x,y width,height). If the viewport was not added
    * before, nothing happens.
    *
    * @see ViewManager
    */
    void erase(View view);

    /**
    * \brief Erase a previously defined viewport using index ind.
    *
    * @param ind Viewport index [0 size()-1]
    */
    void erase(ViewId ind);

    /**
    * \brief replace a previously defined viewport at index ind with a new viewport using the
    * following coordinate system:
    * y ^
    *   |
    *   |
    *   ------> x
    *
    * @see ViewManager
    * @param ind Viewport index [0 size()-1]
    */
    void replace(ViewId ind, View view);

    /** 
     * \brief Return viewport using index ind.
     *
     * @param ind Viewport index [0 size()-1]
     * @return ivec4& 
     */
    View& operator[](ViewId ind);
    size_t size() const;
    void clear();

private:
    struct EventState {
        std::pair<bool, ViewId> getView(ViewManager&  m, const MouseEvent* me) {
            if(!pressing_ && me->buttonState() != MouseButton::None) {       // Start Pressing
                pressing_ = true;
                pressedView_ = m.findView(static_cast<ivec2>(me->pos()));
            } else if (pressing_ && me->buttonState() == MouseButton::None) { // Stop Pressing
                pressing_ = false;
                pressedView_ = {false, 0};
            }
            return pressing_ ? pressedView_ : m.findView(static_cast<ivec2>(me->pos()));
        }

        std::pair<bool, ViewId> getView(ViewManager&  m, const GestureEvent* ge) {
            if (!pressing_ && ge->state() == GestureState::Started) {       // Start Pressing
                pressing_ = true;
                pressedView_ = m.findView(
                    static_cast<ivec2>(dvec2(ge->canvasSize()) * ge->screenPosNormalized()));
            } else if (pressing_ && ge->state() == GestureState::Finished) {  // Stop Pressing
                pressing_ = false;
                auto tmp = pressedView_;
                pressedView_ = { false, 0 };
                return tmp;
            }
            return pressedView_;
        }


        bool pressing_ = false;
        std::pair<bool, ViewId> pressedView_ = {false, 0};
    };

    std::unique_ptr<Event> handlePickingEvent(const PickingEvent* pe); 
    std::unique_ptr<Event> handleMouseEvent(const MouseEvent* me); 
    std::unique_ptr<Event> handleWheelEvent(const WheelEvent* we); 
    std::unique_ptr<Event> handleGestureEvent(const GestureEvent* ge); 
    std::unique_ptr<Event> handleTouchEvent(const TouchEvent* te);

    std::pair<bool, ViewId> findView(ivec2 pos) const;
    static bool inView(const View& view, const ivec2& pos);

    EventState eventState_;
    std::pair<bool, ViewId> selectedView_ = {false, 0};
    ViewList views_;
};

}  // namespace

#endif  // IVW_VIEWMANAGER_H
