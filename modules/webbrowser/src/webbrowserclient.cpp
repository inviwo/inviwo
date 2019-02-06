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

#include <modules/webbrowser/webbrowserclient.h>

#include <warn/push>
#include <warn/ignore/all>
#include <include/wrapper/cef_helpers.h>
#include <warn/pop>

namespace inviwo {

WebBrowserClient::WebBrowserClient(CefRefPtr<RenderHandlerGL> renderHandler)
    : renderHandler_(renderHandler) {}

void WebBrowserClient::SetRenderHandler(CefRefPtr<RenderHandlerGL> renderHandler) {
    renderHandler_ = renderHandler;
}

bool WebBrowserClient::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                                CefProcessId source_process,
                                                CefRefPtr<CefProcessMessage> message) {
    CEF_REQUIRE_UI_THREAD();

    return messageRouter_->OnProcessMessageReceived(browser, source_process, message);
}

void WebBrowserClient::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
    CEF_REQUIRE_UI_THREAD();

    if (!messageRouter_) {
        // Create the browser-side router for query handling.
        CefMessageRouterConfig config;
        messageRouter_ = CefMessageRouterBrowserSide::Create(config);

        // Register handlers with the router.
        propertyCefSynchronizer_ = new PropertyCefSynchronizer();
        messageRouter_->AddHandler(propertyCefSynchronizer_.get(), false);
    }

    browserCount_++;

    // Call the default shared implementation.
    CefLifeSpanHandler::OnAfterCreated(browser);
}

bool WebBrowserClient::DoClose(CefRefPtr<CefBrowser> browser) {
    // Call the default shared implementation.
    return CefLifeSpanHandler::DoClose(browser);
}

void WebBrowserClient::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
    CEF_REQUIRE_UI_THREAD();

    if (--browserCount_ == 0) {
        // Free the router when the last browser is closed.
        messageRouter_->RemoveHandler(propertyCefSynchronizer_.get());
        propertyCefSynchronizer_ = nullptr;
        messageRouter_ = NULL;
    }

    // Call the default shared implementation.
    CefLifeSpanHandler::OnBeforeClose(browser);
}

bool WebBrowserClient::OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                      CefRefPtr<CefRequest> request, bool /*user_gesture*/,
                                      bool /*is_redirect*/) {
    CEF_REQUIRE_UI_THREAD();

    messageRouter_->OnBeforeBrowse(browser, frame);
    return false;
}

void WebBrowserClient::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                                 TerminationStatus status) {
    CEF_REQUIRE_UI_THREAD();
    switch (status) {
        case TS_ABNORMAL_TERMINATION:
            LogError(
                "Web renderer process killed due to non-zero exit status "
                "(TS_ABNORMAL_TERMINATION).");
            break;
        case TS_PROCESS_WAS_KILLED:
            LogError(
                "Web renderer process killed due to SIGKILL or task manager kill "
                "(TS_PROCESS_WAS_KILLED).");
            break;
        case TS_PROCESS_CRASHED:
            LogError(
                "Web renderer process killed due to segmentation fault (TS_ABNORMAL_TERMINATION).");
            break;
        case TS_PROCESS_OOM:
            LogError(
                "Web renderer process killed due to out of memory (TS_PROCESS_OOM). Some platforms "
                "may use TS_PROCESS_CRASHED instead.");
            break;
    }

    messageRouter_->OnRenderProcessTerminated(browser);
}

}  // namespace inviwo
