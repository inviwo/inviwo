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

#ifndef IVW_EVENTCONVERTERQT_H
#define IVW_EVENTCONVERTERQT_H

#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/interaction/events/gestureevent.h>

class QMouseEvent;
class QWheelEvent;
class QInputEvent;
class QGesture;
class QKeyEvent;

namespace inviwo {

class IVW_QTWIDGETS_API EventConverterQt {
public:
    EventConverterQt();
    ~EventConverterQt();

    /** 
     * \brief  Convert the button state
     * when the event was generated 
     * to an inviwo button.
     *
     * Converts QMouseEvent::getButtons().
     *
     * @param QMouseEvent * e 
     * @return MouseEvent::MouseButton 
     */
    static MouseEvent::MouseButton getMouseButton(const QMouseEvent* e);
    /** 
     * \brief Convert the button originally 
     * causing the event to an inviwo button.
     *
     * Converts QMouseEvent::getButton().
     * 
     * @note Qt does not include the button 
     * that caused a release event in the regular 
     * QMouseEvent::getButtons function, which is 
     * why a separate conversion function
     * is necessary 
     * @param QMouseEvent * e 
     * @return MouseEvent::MouseButton 
     */
    static MouseEvent::MouseButton getMouseButtonCausingEvent(const QMouseEvent* e);
    static MouseEvent::MouseButton getMouseWheelButton(const QWheelEvent* e);
    static InteractionEvent::Modifier getModifier(const QInputEvent* e);
#ifndef QT_NO_GESTURES
    static GestureEvent::GestureState getGestureState(const QGesture* e);
#endif
    static int getKeyButton(const QKeyEvent* e);
};
} // namespace
#endif // IVW_EVENTCONVERTERQT_H