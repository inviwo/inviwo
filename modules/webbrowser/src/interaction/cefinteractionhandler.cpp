/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2021 Inviwo Foundation
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
#include <inviwo/core/interaction/events/pickingevent.h>
#include <inviwo/core/interaction/events/resizeevent.h>
#include <inviwo/core/interaction/events/wheelevent.h>
#include <inviwo/core/interaction/events/touchevent.h>

#include <iostream>
#include <string>
#include <locale>
#include <codecvt>

namespace inviwo {

CEFInteractionHandler::CEFInteractionHandler(CefRefPtr<CefBrowserHost> host) : host_(host){};

void CEFInteractionHandler::invokeEvent(Event* event) {
    switch (event->hash()) {
        case ResizeEvent::chash(): {
            if (renderHandler_) {
                auto resizeEvent = static_cast<ResizeEvent*>(event);
                renderHandler_->updateCanvasSize(host_->GetBrowser(), resizeEvent->size());
                host_->WasResized();
            }
            break;
        }
        case KeyboardEvent::chash(): {
            auto keyEvent = event->getAs<KeyboardEvent>();
            auto cefEvent = mapKeyEvent(keyEvent);
            host_->SendKeyEvent(cefEvent);
            // Send CHAR event for characters, but not non-char keys like arrows,
            // function keys or clear.
            auto isCharacter = std::iscntrl(cefEvent.character) == 0;
            if (isCharacter && (keyEvent->state() & KeyState::Press)) {
                cefEvent.type = KEYEVENT_CHAR;
                // Fixes 'Legacy' key codes for keypress events at
                // https://dvcs.w3.org/hg/d4e/raw-file/tip/key-event-test.html
                cefEvent.windows_key_code = cefEvent.character;
                host_->SendKeyEvent(cefEvent);
            }
            event->markAsUsed();
            break;
        }
    }
}

void CEFInteractionHandler::handlePickingEvent(PickingEvent* p) {
    if (p->getEvent()->hash() == MouseEvent::chash()) {
        auto mouseEvent = p->getEventAs<MouseEvent>();
        updateMouseStates(mouseEvent);
        auto cefMouseEvent = mapMouseEvent(mouseEvent);
        if (mouseEvent->state() & MouseState::Move) {
            bool mouseLeave = false;
            host_->SendMouseMoveEvent(cefMouseEvent, mouseLeave);
            p->markAsUsed();
        } else if (mouseEvent->state() & MouseState::Press ||
                   mouseEvent->state() & MouseState::Release) {
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
            p->markAsUsed();
        }
    } else if (auto touchEvent = p->getEventAs<TouchEvent>()) {

        if (!touchEvent->hasTouchPoints()) {
            return;
        }
        TouchDevice::DeviceType type = touchEvent->getDevice()
                                           ? touchEvent->getDevice()->getType()
                                           : TouchDevice::DeviceType::TouchScreen;
        if (type == TouchDevice::DeviceType::TouchPad) {
            // Mouse events are emulated on touch pads for single touch point
            // but we need still need to consume multi-touch events if user pressed on
            p->markAsUsed();
            return;
        }
        const auto& touchPoints = touchEvent->touchPoints();
        for (auto touchPoint : touchPoints) {
            auto cefEvent = mapTouchEvent(&touchPoint, touchEvent->getDevice());
            host_->SendTouchEvent(cefEvent);
        }
        p->markAsUsed();

    } else if (auto wheelEvent = p->getEventAs<WheelEvent>()) {
        auto cefMouseEvent = mapMouseEvent(wheelEvent);
        // cef expects the wheel delta in multiples of 120
        // see https://magpcss.org/ceforum/viewtopic.php?f=6&t=18203
        host_->SendMouseWheelEvent(cefMouseEvent, static_cast<int>(wheelEvent->delta().x * 120),
                                   static_cast<int>(wheelEvent->delta().y * 120));
        p->markAsUsed();
    }
}

CefMouseEvent CEFInteractionHandler::mapMouseEvent(const MouseInteractionEvent* e) {
    CefMouseEvent cefEvent;
    cefEvent.x = static_cast<int>(e->x());
    cefEvent.y = static_cast<int>(e->canvasSize().y) - static_cast<int>(e->y());
    cefEvent.modifiers = modifiers_;
    return cefEvent;
}

CefTouchEvent CEFInteractionHandler::mapTouchEvent(const TouchPoint* p, const TouchDevice* device) {
    CefTouchEvent cefEvent;
    cefEvent.id = p->id();
    // X coordinate relative to the left side of the view.
    cefEvent.x = static_cast<float>(p->pos().x);
    // Y coordinate relative to the top side of the view.
    cefEvent.y = static_cast<float>(p->canvasSize().y - p->pos().y);
    // Radius in pixels. Set to 0 if not applicable.
    cefEvent.radius_x = cefEvent.radius_y = 0;
    // Radius in pixels. Set to 0 if not applicable.
    cefEvent.rotation_angle = 0;
    // The normalized pressure of the pointer input in the range of [0,1].
    cefEvent.pressure = static_cast<float>(p->pressure());
    // The state of the touch point. Touches begin with one CEF_TET_PRESSED event
    // followed by zero or more CEF_TET_MOVED events and finally one
    // CEF_TET_RELEASED or CEF_TET_CANCELLED event. Events not respecting this
    // order will be ignored.
    auto toCefEventType = [](auto state) -> cef_touch_event_type_t {
        switch (state) {
            case TouchState::None:
                return CEF_TET_CANCELLED;
            case TouchState::Started:
                return CEF_TET_PRESSED;
            case TouchState::Updated:
            case TouchState::Stationary:
                return CEF_TET_MOVED;
            case TouchState::Finished:
                return CEF_TET_RELEASED;
            default:  // Incorrect usage or new state added (warnings if left out)
                assert(false);
                return CEF_TET_CANCELLED;
        }
    };

    cefEvent.type = toCefEventType(p->state());
    auto toCefPointerType = [](auto device) -> cef_pointer_type_t {
        switch (device.getType()) {
            case TouchDevice::DeviceType::TouchScreen:
                return CEF_POINTER_TYPE_TOUCH;
            case TouchDevice::DeviceType::TouchPad:
                return CEF_POINTER_TYPE_MOUSE;
                // No types for these ones yet
                // case TouchDevice::DeviceType::Pen:
                //    return CEF_POINTER_TYPE_PEN;
                // case TouchDevice::DeviceType::Eraser:
                //    return CEF_POINTER_TYPE_ERASER;
            default:  // Incorrect usage or new state added (warnings if left out)
                assert(false);
                return CEF_POINTER_TYPE_TOUCH;
        }
    };
    cefEvent.pointer_type = toCefPointerType(*device);

    cefEvent.modifiers = modifiers_;
    return cefEvent;
}

CefKeyEvent CEFInteractionHandler::mapKeyEvent(const KeyboardEvent* e) {
    CefKeyEvent cefEvent;

    // TODO: Fix key code translation to match the ones used in CEF
    // cefEvent.type = e->state() & KeyState::Press ? KEYEVENT_RAWKEYDOWN : KEYEVENT_CHAR;
    cefEvent.type = e->state() & KeyState::Press ? KEYEVENT_KEYDOWN : KEYEVENT_KEYUP;
    // Convert character to UTF16
#if _MSC_VER
    // Linker error when using char16_t in visual studio
    // https://social.msdn.microsoft.com/Forums/vstudio/en-US/8f40dcd8-c67f-4eba-9134-a19b9178e481/vs-2015-rc-linker-stdcodecvt-error?forum=vcgeneral
    auto textUTF16 = std::wstring_convert<std::codecvt_utf8_utf16<uint16_t>, uint16_t>{}.from_bytes(
        e->text().data());
#else
    auto textUTF16 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(
        e->text().data());
#endif

    if (textUTF16.length() > 0) {
        cefEvent.character = textUTF16[0];
    } else {
        cefEvent.character = 0;
    }

    // Tested on Windows. You can test keys at
    // https://dvcs.w3.org/hg/d4e/raw-file/tip/key-event-test.html
    // Compare the result with typing in Chrome
    cefEvent.windows_key_code = e->getNativeVirtualKey();
    // TODO: Get correct native_key_code, i.e. event LParam on Windows.
    // We might need the ScanCode sent by the system/Qt to get this.
    // An alternative would be to convert our the KeyEvent according to the Java KeyEvent, which
    // seem to be used by Chromium. See
    // https://docs.oracle.com/javase/6/docs/api/java/awt/event/KeyEvent.html and
    // https://www.w3.org/TR/uievents-key/#named-key-attribute-values
    //
    // The native_key_code will be translated to DOM3 'code'
    // https://chromium.googlesource.com/chromium/src/+/master/ui/events/keycodes/dom/keycode_converter.h
    // According to this:
    // https://chromium.googlesource.com/chromium/src/+/master/ui/events/keycodes/dom/dom_key_data.inc
    // Note: native_key_code currently has no effect on text input fields, so text input will
    // display correctly.
    // cefEvent.native_key_code = TODO;

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
void CEFInteractionHandler::updateMouseStates(TouchEvent* e) {
    if (e->touchPoints().front().state() & TouchState::Finished) {
        // Remove modifiers
        modifiers_ &= ~(EVENTFLAG_LEFT_MOUSE_BUTTON | EVENTFLAG_MIDDLE_MOUSE_BUTTON |
                        EVENTFLAG_MIDDLE_MOUSE_BUTTON);
    } else {
        // Add modifiers
        modifiers_ |= (EVENTFLAG_LEFT_MOUSE_BUTTON);
    }
}
};  // namespace inviwo
