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

#ifndef IVW_WEBBROWSERCLIENT_H
#define IVW_WEBBROWSERCLIENT_H

#include <modules/webbrowser/webbrowsermoduledefine.h>
#include <modules/webbrowser/renderhandlergl.h>
#include <modules/webbrowser/properties/propertycefsynchronizer.h>

#include <inviwo/core/common/inviwoapplication.h>

#include <warn/push>
#include <warn/ignore/all>
#include <include/cef_client.h>
#include <include/cef_load_handler.h>
#include <include/cef_life_span_handler.h>
#include "include/wrapper/cef_message_router.h"
#include <warn/pop>

namespace inviwo {

/* \class WebBrowserClient
 * CefClient with custom render handler
 */
class WebBrowserClient : public CefClient, public CefLifeSpanHandler, public CefRequestHandler {
public:
    WebBrowserClient(CefRefPtr<RenderHandlerGL> renderHandler);

    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override { return propertyCefSynchronizer_; }
    virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override { return renderHandler_; }
    virtual CefRefPtr<CefRequestHandler> GetRequestHandler() override { return this; }

    void SetRenderHandler(CefRefPtr<RenderHandlerGL> renderHandler);

    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }

    bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process,
                                  CefRefPtr<CefProcessMessage> message) override;

    // CefLifeSpanHandler methods:
    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    bool DoClose(CefRefPtr<CefBrowser> browser) override;
    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    // CefRequestHandler methods:
    virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefRequest> request, bool user_gesture,
                                bool is_redirect) override;

    void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                   TerminationStatus status) override;

    CefRefPtr<PropertyCefSynchronizer> propertyCefSynchronizer_;

protected:
    InviwoApplication* app_;
    CefRefPtr<CefRenderHandler> renderHandler_;
    // Handles the browser side of query routing.
    CefRefPtr<CefMessageRouterBrowserSide> messageRouter_;

    // Track the number of browsers using this Client.
    int browserCount_ = 0;

private:
    IMPLEMENT_REFCOUNTING(WebBrowserClient)
    DISALLOW_COPY_AND_ASSIGN(WebBrowserClient);
};

}  // namespace inviwo

#endif  // IVW_WEBBROWSERCLIENT_H
