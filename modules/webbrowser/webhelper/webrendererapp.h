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

#ifndef IVW_WEBBROWSERAPP_H
#define IVW_WEBBROWSERAPP_H

#include <warn/push>
#include <warn/ignore/all>
#include <include/cef_app.h>
#include <include/wrapper/cef_helpers.h>
#include "include/wrapper/cef_message_router.h"
#include <warn/pop>

namespace inviwo {

/**
 * App to be used in the renderer thread (web_helper). Handles message routing for javascript.
 */
class WebRendererApp : public CefApp, public CefRenderProcessHandler {
public:
    WebRendererApp();

    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override;

    // CefRenderProcessHandler methods:
    void OnWebKitInitialized() OVERRIDE {
        // Create the renderer-side router for query handling.
        CefMessageRouterConfig config;
        messageRouter_ = CefMessageRouterRendererSide::Create(config);
    }

    void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                          CefRefPtr<CefV8Context> context) OVERRIDE {
        // Register the JavaScripts functions with the new context.
        messageRouter_->OnContextCreated(browser, frame, context);
    }

    void OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                           CefRefPtr<CefV8Context> context) OVERRIDE {

        messageRouter_->OnContextReleased(browser, frame, context);
    }

    bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process,
                                  CefRefPtr<CefProcessMessage> message) OVERRIDE {
        return messageRouter_->OnProcessMessageReceived(browser, source_process, message);
    }

private:
    // Handles the renderer side of query routing.
    // Adds JavaScript function "cefQuery" to the 'window' object for sending a query.
    CefRefPtr<CefMessageRouterRendererSide> messageRouter_;
    IMPLEMENT_REFCOUNTING(WebRendererApp);
    DISALLOW_COPY_AND_ASSIGN(WebRendererApp);
};

};      // namespace inviwo
#endif  // IVW_WEBBROWSERAPP_H
