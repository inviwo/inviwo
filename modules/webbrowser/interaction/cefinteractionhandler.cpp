/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

#include <modules/webbrowser/interaction/cefinteractionhandler.h>
#include <modules/webbrowser/interaction/cefkeyboardmapping.h>
#include <modules/webbrowser/renderhandlergl.h>
#include <inviwo/core/interaction/events/resizeevent.h>
#include <inviwo/core/interaction/events/wheelevent.h>
#include <inviwo/core/interaction/events/touchevent.h>

namespace inviwo {

CEFInteractionHandler::CEFInteractionHandler(CefRefPtr<CefBrowserHost> host) : host_(host){};

void CEFInteractionHandler::invokeEvent(Event* event) {
    switch (event->hash()) {
        case ResizeEvent::chash(): {
            auto resizeEvent = static_cast<ResizeEvent*>(event);
            renderHandler_->updateCanvasSize(resizeEvent->size());
            host_->WasResized();
            break;
        }
        case KeyboardEvent::chash(): {
            host_->SendKeyEvent(mapKeyEvent(static_cast<KeyboardEvent*>(event)));
            event->markAsUsed();
            break;
        }
        case MouseEvent::chash(): {
            auto mouseEvent = static_cast<MouseEvent*>(event);
            // Only handle events if pressed on non-transparent pixels
            auto pixel = renderHandler_->getPixel(static_cast<int>(mouseEvent->x()), static_cast<int>(mouseEvent->y()));
            if (mouseEvent->state() == MouseState::Press) {
                if (pixel.w != 0) {
                    pressedNonTransparentPixel = true;
                } else {
                    pressedNonTransparentPixel = false;
                }
            }
            updateMouseStates(mouseEvent);
            auto cefMouseEvent = mapMouseEvent(mouseEvent);
            if (MouseState::Move & mouseEvent->state() &&
                (pressedNonTransparentPixel || pixel.w != 0)) {
                bool mouseLeave = false;
                host_->SendMouseMoveEvent(cefMouseEvent, mouseLeave);
                event->markAsUsed();
            } else if (pressedNonTransparentPixel) {
                CefBrowserHost::MouseButtonType type;
                
                if (mouseEvent->button() & MouseButton::Left) {
                    type = MBT_LEFT;
                } else if (mouseEvent->button() & MouseButton::Middle) {
                    type = MBT_MIDDLE;
                } else {  // if (mouseEvent->button() & MouseButton::Right) {
                    type = MBT_RIGHT;
                }
                bool mouseUp = MouseState::Release & mouseEvent->state() ? true : false;
                int clickCount = MouseState::DoubleClick & mouseEvent->state() ? 2 : 1;
                host_->SendMouseClickEvent(cefMouseEvent, type, mouseUp, clickCount);
                event->markAsUsed();
            }
            if (MouseState::Release & mouseEvent->state()) {
                // Clear flag
                pressedNonTransparentPixel = false;
            }
            break;
        }
        case TouchEvent::chash(): {
            auto touchEvent = static_cast<TouchEvent*>(event);
            if (!touchEvent->hasTouchPoints()) {
                break;
            }
            TouchDevice::DeviceType type = touchEvent->getDevice() ? touchEvent->getDevice()->getType() : TouchDevice::DeviceType::TouchScreen;
            if (type == TouchDevice::DeviceType::TouchPad) {
                // Mouse events are emulated on touch pads for single touch point
                // but we need to consume multi-touch events if user pressed on
                // non-transparent pixel
                if (pressedNonTransparentPixel) {
                    event->markAsUsed();
                }
                break;
                
            }
            const auto& touchPoints = touchEvent->touchPoints();

            // Only single touch point is currently supported
            const auto& activeTouchPoint = touchPoints[0];
            TouchStates moveState = (TouchState::Updated | TouchState::Stationary);
            TouchStates pressState = (TouchState::Started | TouchState::Finished);
            
            // Only handle events if pressed on non-transparent pixels
            auto pixel = renderHandler_->getPixel(static_cast<int>(activeTouchPoint.pos().x), static_cast<int>(activeTouchPoint.pos().y));
            if (activeTouchPoint.state() == TouchState::Started) {
                if (pixel.w != 0) {
                    pressedNonTransparentPixel = true;
                } else {
                    pressedNonTransparentPixel = false;
                }
            }
            auto cefEvent = mapTouchEvent(&activeTouchPoint);
            if (activeTouchPoint.state() & moveState &&
                (pressedNonTransparentPixel || pixel.w != 0)) {
                bool mouseLeave = false;
                host_->SendMouseMoveEvent(cefEvent, mouseLeave);
                event->markAsUsed();
            } else if (activeTouchPoint.state() & pressState) {
                // Emulate mouse press
                CefBrowserHost::MouseButtonType button;
                button = MBT_LEFT;
                bool mouseUp = activeTouchPoint.state() & (TouchState::Finished) ? true : false;
                int clickCount = 1;
                host_->SendMouseClickEvent(cefEvent, button, mouseUp, clickCount);
                event->markAsUsed();
            }
            if (TouchState::Finished & activeTouchPoint.state()) {
                // Clear flag
                pressedNonTransparentPixel = false;
            }
        }
        case WheelEvent::chash(): {
            auto wheelEvent = static_cast<WheelEvent*>(event);
            auto pixel = renderHandler_->getPixel(static_cast<int>(wheelEvent->x()), static_cast<int>(wheelEvent->y()));
            if (pixel.w != 0) {
                auto cefMouseEvent = mapMouseEvent(wheelEvent);
                host_->SendMouseWheelEvent(cefMouseEvent, static_cast<int>(wheelEvent->delta().x),
                                           static_cast<int>(wheelEvent->delta().y));
                event->markAsUsed();
            }
            break;
        }
    }
}

CefMouseEvent CEFInteractionHandler::mapMouseEvent(const MouseInteractionEvent* e) {
    CefMouseEvent cefEvent;
    cefEvent.x = static_cast<int>(e->x());
    cefEvent.y = static_cast<int>(e->canvasSize().y) - static_cast<int>(e->y());
    cefEvent.modifiers = modifiers_;
    return cefEvent;
}
    
CefMouseEvent CEFInteractionHandler::mapTouchEvent(const TouchPoint* p) {
    CefMouseEvent cefEvent;
    cefEvent.x = static_cast<int>(p->pos().x);
    cefEvent.y = static_cast<int>(p->canvasSize().y) - static_cast<int>(p->pos().y);
    cefEvent.modifiers = modifiers_;
    return cefEvent;
}

CefKeyEvent CEFInteractionHandler::mapKeyEvent(const KeyboardEvent* e) {
    CefKeyEvent cefEvent;

    // TODO: Fix key code translation to match the ones used in CEF
    cefEvent.type = e->state() & KeyState::Press ? KEYEVENT_RAWKEYDOWN : KEYEVENT_CHAR;
    // cefEvent.type = e->state() & KeyState::Press ? KEYEVENT_KEYDOWN : KEYEVENT_KEYUP;

    cefEvent.windows_key_code = cef::mapKey(e->key());

    // cefEvent.native_key_code = e->nativeVirtualKey_;
    // cefEvent.native_key_code = static_cast<int>(e->key());
#ifdef _WINDOWS
    // F10 or ALT
    // https://msdn.microsoft.com/en-us/library/ms646286(VS.85).aspx
    cefEvent.is_system_key = (e->key() == IvwKey::F10) || (e->modifiers() & KeyModifier::Alt);
#else
    // Always false on non-windows platforms
    cefEvent.is_system_key = false;
#endif
    if (e->state() & KeyState::Press) {
        modifiers_ |= cef::keyModifiers(e->modifiers(), e->key());
    } else {
        modifiers_ &= ~cef::keyModifiers(e->modifiers(), e->key());
    }
    cefEvent.modifiers = modifiers_;
    return cefEvent;
}

void CEFInteractionHandler::updateMouseStates(MouseEvent* e) {
    if (e->state() & MouseState::Release) {
        // Remove modifiers
        modifiers_ &= ~(EVENTFLAG_LEFT_MOUSE_BUTTON | EVENTFLAG_MIDDLE_MOUSE_BUTTON |
                        EVENTFLAG_MIDDLE_MOUSE_BUTTON);
    } else {
        // Add modifiers
        modifiers_ |= (e->button() & MouseButton::Left ? EVENTFLAG_LEFT_MOUSE_BUTTON : 0) |
                      (e->button() & MouseButton::Middle ? EVENTFLAG_MIDDLE_MOUSE_BUTTON : 0) |
                      (e->button() & MouseButton::Right ? EVENTFLAG_RIGHT_MOUSE_BUTTON : 0);
    }
}
};  // namespace inviwo
