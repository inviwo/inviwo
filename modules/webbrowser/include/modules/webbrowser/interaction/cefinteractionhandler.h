/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#ifndef IVW_CEFINTERACTIONHANDLER_H
#define IVW_CEFINTERACTIONHANDLER_H

#include <modules/webbrowser/webbrowsermoduledefine.h>
#include <inviwo/core/interaction/interactionhandler.h>
#include <inviwo/core/interaction/events/keyboardevent.h>
#include <inviwo/core/interaction/events/mouseevent.h>

#include <warn/push>
#include <warn/ignore/all>
#include <include/cef_browser.h>
#include <warn/pop>

namespace inviwo {
class RenderHandlerGL;
class TouchPoint;
class TouchEvent;
class PickingEvent;
/*\class CEFInteractionHandler
 * Translates Inviwo events to CEF events and injects them into provided CefBrowserHost.
 * Assumes that PickingEvent is sent for mouse and touch events
 */
class IVW_MODULE_WEBBROWSER_API CEFInteractionHandler : public InteractionHandler {
public:
    /*
     * host must be provided before invokeEvent is called.
     * @param host The browser to inject events into.
     */
    CEFInteractionHandler(CefRefPtr<CefBrowserHost> host = nullptr);
    virtual ~CEFInteractionHandler() = default;

    virtual std::string getClassIdentifier() const override {
        return "org.inviwo.cefinteractionhandler";
    }

    virtual void invokeEvent(Event* event) override;

    void handlePickingEvent(PickingEvent* p);

    void setHost(CefRefPtr<CefBrowserHost> host) { host_ = host; }
    CefRefPtr<CefBrowserHost> getHost() const { return host_; }
    /*
     * Render hander to forward resize events to.
     * Will do nothing if null and does not take ownership
     */
    void setRenderHandler(RenderHandlerGL* renderHandler) { renderHandler_ = renderHandler; }
    RenderHandlerGL* getRenderHandler() const { return renderHandler_; }

private:
    CefKeyEvent mapKeyEvent(const KeyboardEvent* e);
    CefMouseEvent mapMouseEvent(const MouseInteractionEvent* e);
    CefMouseEvent mapTouchEvent(const TouchPoint* p);

    void updateMouseStates(MouseEvent* e);
    void updateMouseStates(TouchEvent* e);
    uint32 modifiers_ = 0;
    CefRefPtr<CefBrowserHost> host_;
    RenderHandlerGL* renderHandler_ = nullptr;  ///< Forward resize event if set
};

};  // namespace inviwo

#endif  // IVW_CEFINTERACTIONHANDLER_H
