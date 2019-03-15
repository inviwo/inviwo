/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>

#include <inviwo/core/interaction/events/keyboardkeys.h>
#include <inviwo/core/interaction/events/mousebuttons.h>
#include <inviwo/core/interaction/events/gesturestate.h>

class QMouseEvent;
class QWheelEvent;
class QInputEvent;
class QGesture;
class QKeyEvent;

namespace inviwo {

namespace utilqt {
/**
 * \brief  Convert the button state
 * when the event was generated
 * to an inviwo button.
 *
 * Converts QMouseEvent::getButtons().
 */
IVW_MODULE_QTWIDGETS_API MouseButtons getMouseButtons(const QMouseEvent* e);

/**
 * \brief Convert the button originally
 * causing the event to an inviwo button.
 *
 * Converts QMouseEvent::getButton().
 *
 * @note Qt does not include the button that caused a release event in the regular
 * QMouseEvent::getButtons function, which is why a separate conversion function is necessary
 * @param e the QT Mouse Event
 * @return MouseEvent::MouseButton
 */
IVW_MODULE_QTWIDGETS_API MouseButton getMouseButtonCausingEvent(const QMouseEvent* e);
IVW_MODULE_QTWIDGETS_API MouseButtons getMouseWheelButtons(const QWheelEvent* e);
IVW_MODULE_QTWIDGETS_API KeyModifiers getModifiers(const QInputEvent* e);
IVW_MODULE_QTWIDGETS_API GestureState getGestureState(const QGesture* e);

IVW_MODULE_QTWIDGETS_API IvwKey getKeyButton(const QKeyEvent* e);

}  // namespace utilqt

}  // namespace inviwo

#endif  // IVW_EVENTCONVERTERQT_H