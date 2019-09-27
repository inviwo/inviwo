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

#include <modules/webbrowser/webbrowserclient.h>
#include <modules/webbrowser/webbrowsermodule.h>

#include <warn/push>
#include <warn/ignore/all>
#include <include/wrapper/cef_helpers.h>
#include <warn/pop>

namespace inviwo {

namespace detail {

// Register inviwo application and module directories for resource loading
void setupResourceManager(CefRefPtr<CefResourceManager> resource_manager) {
    if (!CefCurrentlyOn(TID_IO)) {
        // Execute on the browser IO thread.
        CefPostTask(TID_IO, base::Bind(setupResourceManager, resource_manager));
        return;
    }
    std::string origin = "https://inviwo";
    // Redirect paths to corresponding app/module directories.
    // Enables resource loading from these directories directory (js-files and so on).
    auto appOrigin = origin + "/app";
    resource_manager->AddDirectoryProvider(appOrigin, InviwoApplication::getPtr()->getBasePath(),
                                           99, std::string());

    auto moduleOrigin = origin + "/modules";
    for (const auto& m : InviwoApplication::getPtr()->getModules()) {
        auto mOrigin = moduleOrigin + "/" + toLower(m->getIdentifier());
        auto moduleDir = m->getPath();
        resource_manager->AddDirectoryProvider(mOrigin, moduleDir, 100, std::string());
    }
}

}  // namespace detail

WebBrowserClient::WebBrowserClient(CefRefPtr<RenderHandlerGL> renderHandler,
                                   const PropertyWidgetCEFFactory* widgetFactory)
    : widgetFactory_(widgetFactory)
    , renderHandler_(renderHandler)
    , resourceManager_(new CefResourceManager()) {
    detail::setupResourceManager(resourceManager_);
}

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

        // Register handlers with the router
        propertyCefSynchronizer_ = new PropertyCefSynchronizer(widgetFactory_);
        addLoadHandler(propertyCefSynchronizer_);
        messageRouter_->AddHandler(propertyCefSynchronizer_.get(), false);
        processorCefSynchronizer_ = new ProcessorCefSynchronizer();
        addLoadHandler(processorCefSynchronizer_);
        messageRouter_->AddHandler(processorCefSynchronizer_.get(), false);
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
        removeLoadHandler(propertyCefSynchronizer_);
        propertyCefSynchronizer_ = nullptr;
        messageRouter_->RemoveHandler(processorCefSynchronizer_.get());
        removeLoadHandler(processorCefSynchronizer_);
        processorCefSynchronizer_ = nullptr;

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

cef_return_value_t WebBrowserClient::OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
                                                          CefRefPtr<CefFrame> frame,
                                                          CefRefPtr<CefRequest> request,
                                                          CefRefPtr<CefRequestCallback> callback) {
    CEF_REQUIRE_IO_THREAD();

    return resourceManager_->OnBeforeResourceLoad(browser, frame, request, callback);
}

CefRefPtr<CefResourceHandler> WebBrowserClient::GetResourceHandler(CefRefPtr<CefBrowser> browser,
                                                                   CefRefPtr<CefFrame> frame,
                                                                   CefRefPtr<CefRequest> request) {
    CEF_REQUIRE_IO_THREAD();

    return resourceManager_->GetResourceHandler(browser, frame, request);
}

void WebBrowserClient::addLoadHandler(CefLoadHandler* loadHandler) {
    loadHandlers_.emplace_back(loadHandler);
}

void WebBrowserClient::removeLoadHandler(CefLoadHandler* loadHandler) {
    util::erase_remove(loadHandlers_, loadHandler);
}

void WebBrowserClient::OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading,
                                            bool canGoBack, bool canGoForward) {
    for (const auto& loadHandler : loadHandlers_) {
        loadHandler->OnLoadingStateChange(browser, isLoading, canGoBack, canGoForward);
    }
}

void WebBrowserClient::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                 int httpStatusCode) {
    for (const auto& loadHandler : loadHandlers_) {
        loadHandler->OnLoadEnd(browser, frame, httpStatusCode);
    }
}

void WebBrowserClient::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                   CefLoadHandler::ErrorCode errorCode, const CefString& errorText,
                                   const CefString& failedUrl) {
    for (const auto& loadHandler : loadHandlers_) {
        loadHandler->OnLoadError(browser, frame, errorCode, errorText, failedUrl);
    }
    if (errorCode == ERR_ABORTED) {
        // Ignore page loading aborted (occurs during deserialization).
        // Prevents error page from showing after deserialization.
        return;
    }
    std::stringstream ss;
    ss << "<html><head><title>Page failed to load</title></head>"
          "<body bgcolor=\"white\">"
          "<h3>Page failed to load.</h3>"
          "URL: <a href=\""
       << failedUrl.ToString() << "\">" << failedUrl.ToString()
       << "</a><br/>Error: " << WebBrowserModule::getCefErrorString(errorCode) << " (" << errorCode
       << ")";

    if (!errorText.empty()) {
        ss << "<br/>Description: " << errorText.ToString();
    }
    ss << "</body></html>";

    frame->LoadURL(WebBrowserModule::getDataURI(ss.str(), "text/html"));
}

bool WebBrowserClient::OnConsoleMessage(CefRefPtr<CefBrowser> browser, cef_log_severity_t level,
                                        const CefString& message, const CefString& source,
                                        int line) {

    if (auto lc = LogCentral::getPtr()) {
        std::string src = source.ToString();

        std::string file = "";
        if (src.rfind("file://", 0) == 0) {
            replaceInString(src, "\\", "/");
            file = splitString(src, '/').back();
        }

        LogLevel loglevel;
        switch (level) {
            case LOGSEVERITY_DISABLE:
                return false;
            case LOGSEVERITY_ERROR:
                loglevel = LogLevel::Error;
                break;
            case LOGSEVERITY_WARNING:
                loglevel = LogLevel::Warn;
                break;
            case cef_log_severity_t::LOGSEVERITY_DEBUG:
            case cef_log_severity_t::LOGSEVERITY_INFO:
            case cef_log_severity_t::LOGSEVERITY_DEFAULT:
            default:
                loglevel = LogLevel::Info;
                break;
        }

        lc->log("WebBrowserClient", loglevel, LogAudience::Developer, file.c_str(), "", line,
                message.ToString());
        return true;
    }
    return false;
}

}  // namespace inviwo
