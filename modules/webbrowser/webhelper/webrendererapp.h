/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2025 Inviwo Foundation
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

#pragma once

#include <warn/push>
#include <warn/ignore/all>
#include "include/wrapper/cef_message_router.h"
#include <include/base/cef_macros.h>             // for DISALLOW_COPY_AND_ASSIGN
#include <include/base/cef_scoped_refptr.h>      // for scoped_refptr
#include <include/cef_app.h>                     // for CefApp
#include <include/cef_base.h>                    // for CefRefPtr, IMPLEMENT_REFCOUNTING
#include <include/cef_browser.h>                 // for CefBrowser
#include <include/cef_frame.h>                   // for CefFrame
#include <include/cef_process_message.h>         // for CefProcessMessage, CefProcessId
#include <include/cef_render_process_handler.h>  // for CefRenderProcessHandler
#include <include/cef_v8.h>                      // for CefV8Context

#include <warn/pop>

namespace inviwo {

/**
 * App to be used in the renderer thread (web_helper). Handles message routing for javascript.
 */
class CefWebRendererApp : public CefApp, public CefRenderProcessHandler {
public:
    CefWebRendererApp();

    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override;

    // CefRenderProcessHandler methods:
    void OnWebKitInitialized() override {
        // Create the renderer-side router for query handling.
        CefMessageRouterConfig config;
        messageRouter_ = CefMessageRouterRendererSide::Create(config);
    }

    void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                          CefRefPtr<CefV8Context> context) override {
        // Register the JavaScripts functions with the new context.
        messageRouter_->OnContextCreated(browser, frame, context);
    }

    void OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                           CefRefPtr<CefV8Context> context) override {

        messageRouter_->OnContextReleased(browser, frame, context);
    }

    bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                  CefProcessId source_process,
                                  CefRefPtr<CefProcessMessage> message) override {
        return messageRouter_->OnProcessMessageReceived(browser, frame, source_process, message);
    }

private:
    // Handles the renderer side of query routing.
    // Adds JavaScript function "cefQuery" to the 'window' object for sending a query.
    CefRefPtr<CefMessageRouterRendererSide> messageRouter_;
    IMPLEMENT_REFCOUNTING(CefWebRendererApp);
    DISALLOW_COPY_AND_ASSIGN(CefWebRendererApp);
};

};  // namespace inviwo