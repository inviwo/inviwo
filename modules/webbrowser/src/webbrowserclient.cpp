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

#include <modules/webbrowser/webbrowserclient.h>

#include <inviwo/core/common/inviwoapplication.h>  // for InviwoApplication
#include <inviwo/core/common/inviwomodule.h>       // for InviwoModule
#include <inviwo/core/common/modulemanager.h>      // for ModuleManager

#include <inviwo/core/util/exception.h>          // for Exception
#include <inviwo/core/util/logcentral.h>         // for LogCentral, LogError
#include <inviwo/core/util/stdextensions.h>      // for erase_remove
#include <inviwo/core/util/stringconversion.h>   // for replaceInString, spl...
#include <modules/webbrowser/renderhandlergl.h>  // for RenderHandlerGL, Ren...
#include <modules/webbrowser/webbrowserutil.h>   // for CefString, cef_log_s...

#include <modules/webbrowser/networkcefsynchronizer.h>

#include <ostream>      // for operator<<, basic_os...
#include <string_view>  // for string_view
#include <utility>      // for pair

#include <algorithm>
#include <warn/push>
#include <warn/ignore/all>
#include <fmt/core.h>                        // for format
#include <include/base/cef_bind.h>           // for Bind
#include <include/base/cef_logging.h>        // for COMPACT_GOOGLE_LOG_D...
#include <include/base/cef_scoped_refptr.h>  // for scoped_refptr
#include <include/cef_base.h>                // for CefRefPtr
#include <include/cef_browser.h>             // for CefBrowser
#include <include/cef_frame.h>               // for CefFrame
#include <include/cef_life_span_handler.h>   // for CefLifeSpanHandler
#include <include/cef_load_handler.h>        // for CefLoadHandler, CefL...
#include <include/cef_process_message.h>     // for CefProcessMessage
#include <include/cef_request.h>             // for CefRequest
#include <include/cef_request_handler.h>     // for CefRequestHandler::T...

#include <include/cef_resource_request_handler.h>  // for CefResourceRequestHa...
#include <include/cef_task.h>                      // for CefCurrentlyOn
#include <include/wrapper/cef_closure_task.h>      // for CefPostTask
#include <include/wrapper/cef_helpers.h>           // for CEF_REQUIRE_UI_THREAD
#include <include/wrapper/cef_resource_manager.h>  // for CefResourceManager
#include <include/wrapper/cef_stream_resource_handler.h>

#include <warn/pop>

class CefResourceHandler;

namespace inviwo {

class PropertyWidgetCEFFactory;

class StringResourceProvider : public CefResourceManager::Provider {

public:
    explicit StringResourceProvider() : CefResourceManager::Provider{} {}

    bool OnRequest(scoped_refptr<CefResourceManager::Request> request) override {
        const std::string& url = request->url();

        if (!url.starts_with("https://inviwo/app/static")) {
            return false;
        }

        const auto id = request->browser()->GetIdentifier();
        if (auto it = resourceHandlers_.find(id); it != resourceHandlers_.end()) {
            return it->second(url, request);
        } else {
            return false;
        }
    }

    void addHandler(
        int id, std::function<bool(const std::string&, scoped_refptr<CefResourceManager::Request>)>
                    handler) {
        resourceHandlers_[id] = handler;
    }
    void removeHandler(int id) { resourceHandlers_.erase(id); }

private:
    std::map<int,
             std::function<bool(const std::string&, scoped_refptr<CefResourceManager::Request>)>>
        resourceHandlers_;
};

namespace detail {

// Register inviwo application and module directories for resource loading
void setupResourceManager(CefRefPtr<CefResourceManager> resourceManager, InviwoApplication* app) {
    if (!CefCurrentlyOn(TID_IO)) {
        // Execute on the browser IO thread.
        CefPostTask(TID_IO, base::BindRepeating(setupResourceManager, resourceManager, app));
        return;
    }
    // Redirect paths to corresponding app/module directories.
    // Enables resource loading from these directories directory (js-files and so on).
    for (const auto& m : app->getModuleManager().getInviwoModules()) {
        const auto moduleOrigin = "https://inviwo/" + toLower(m.getIdentifier());
        const auto moduleDir = m.getPath();
        resourceManager->AddDirectoryProvider(moduleOrigin, moduleDir.string(), 100, std::string());
    }
}

}  // namespace detail

WebBrowserClient::WebBrowserClient(InviwoApplication* app)
    : renderHandler_{new RenderHandlerGL{}}
    , router_{CefMessageRouterBrowserSide::Create(CefMessageRouterConfig{})}
    , networkSync_{new NetWorkCefSynchronizer{app}}
    , resourceManager_{new CefResourceManager{}}
    , stringResourceProvider_{std::make_unique<StringResourceProvider>()} {

    resourceManager_->AddProvider(stringResourceProvider_.get(), 50, "StringResourceProvider");
    resourceManager_->AddDirectoryProvider("https://inviwo/app/", app->getBasePath().string(), 99,
                                           std::string());
    onModulesRegisteredCallback_ = app->getModuleManager().onModulesDidRegister([&, app]() {
        // Ensure that all module resources have been registered before setting up resources
        detail::setupResourceManager(resourceManager_, app);
    });

    router_->AddHandler(networkSync_.get(), false);

    addLoadHandler(networkSync_.get());
}

WebBrowserClient::~WebBrowserClient() = default;

void WebBrowserClient::addStaticHandler(
    int browserId,
    std::function<bool(const std::string&, scoped_refptr<CefResourceManager::Request>)> handler) {

    stringResourceProvider_->addHandler(browserId, std::move(handler));
}
void WebBrowserClient::removeStaticHandler(int browserId) {
    stringResourceProvider_->removeHandler(browserId);
}

NetWorkCefSynchronizer::CallbackHandle WebBrowserClient::registerCallback(
    CefRefPtr<CefBrowser> browser, const std::string& name,
    std::function<NetWorkCefSynchronizer::CallbackFunc> callback) {
    CEF_REQUIRE_UI_THREAD();

    return networkSync_->registerCallback(browser, name, callback);
}

bool WebBrowserClient::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                                CefRefPtr<CefFrame> frame,
                                                CefProcessId source_process,
                                                CefRefPtr<CefProcessMessage> message) {
    CEF_REQUIRE_UI_THREAD();
    return router_->OnProcessMessageReceived(browser, frame, source_process, message);
}

void WebBrowserClient::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
    CEF_REQUIRE_UI_THREAD();

    CefLifeSpanHandler::OnAfterCreated(browser);
}

bool WebBrowserClient::DoClose(CefRefPtr<CefBrowser> browser) {
    // Call the default shared implementation.
    return CefLifeSpanHandler::DoClose(browser);
}

void WebBrowserClient::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
    CEF_REQUIRE_UI_THREAD();

    networkSync_->clear(browser);

    // Call the default shared implementation.
    CefLifeSpanHandler::OnBeforeClose(browser);
}

CefRefPtr<CefResourceRequestHandler> WebBrowserClient::GetResourceRequestHandler(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request,
    [[maybe_unused]] bool is_navigation, [[maybe_unused]] bool is_download,
    [[maybe_unused]] const CefString& request_initiator,
    [[maybe_unused]] bool& disable_default_handling) {
    return this;
}

bool WebBrowserClient::OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                      CefRefPtr<CefRequest> request, bool /*user_gesture*/,
                                      bool /*is_redirect*/) {
    CEF_REQUIRE_UI_THREAD();

    router_->OnBeforeBrowse(browser, frame);
    return false;
}

void WebBrowserClient::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                                 TerminationStatus status, int,
                                                 const CefString& error_string) {
    CEF_REQUIRE_UI_THREAD();
    switch (status) {
        case TS_ABNORMAL_TERMINATION:
            LogError(
                "Web renderer process killed due to non-zero exit status "
                "(TS_ABNORMAL_TERMINATION).\n"
                << error_string);
            break;
        case TS_PROCESS_WAS_KILLED:
            LogError(
                "Web renderer process killed due to SIGKILL or task manager kill "
                "(TS_PROCESS_WAS_KILLED).\n"
                << error_string);
            break;
        case TS_PROCESS_CRASHED:
            LogError(
                "Web renderer process killed due to segmentation fault (TS_ABNORMAL_TERMINATION).\n"
                << error_string);
            break;
        case TS_PROCESS_OOM:
            LogError(
                "Web renderer process killed due to out of memory (TS_PROCESS_OOM). Some platforms "
                "may use TS_PROCESS_CRASHED instead.\n"
                << error_string);
            break;
        case TS_LAUNCH_FAILED:
            LogError(
                "Web renderer process killed due child process never launched. "
                "(TS_LAUNCH_FAILED).\n"
                << error_string);
            break;
        case TS_INTEGRITY_FAILURE:
            LogError(
                "Web renderer process killed by the OS due to code integrity failure "
                "(TS_INTEGRITY_FAILURE).\n"
                << error_string);
            break;
    }

    router_->OnRenderProcessTerminated(browser);
}

void WebBrowserClient::addLoadHandler(CefLoadHandler* loadHandler) {
    loadHandlers_.emplace_back(loadHandler);
}

void WebBrowserClient::removeLoadHandler(CefLoadHandler* loadHandler) {
    std::erase(loadHandlers_, loadHandler);
}

void WebBrowserClient::OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading,
                                            bool canGoBack, bool canGoForward) {
    for (const auto& loadHandler : loadHandlers_) {
        loadHandler->OnLoadingStateChange(browser, isLoading, canGoBack, canGoForward);
    }
}

void WebBrowserClient::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                   TransitionType transition_type) {
    for (const auto& loadHandler : loadHandlers_) {
        loadHandler->OnLoadStart(browser, frame, transition_type);
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
       << "</a><br/>Error: " << cefutil::getCefErrorString(errorCode) << " (" << errorCode << ")";

    if (!errorText.empty()) {
        ss << "<br/>Description: " << errorText.ToString();
    }
    ss << "</body></html>";

    frame->LoadURL(cefutil::getDataURI(ss.str(), "text/html"));
}

bool WebBrowserClient::OnConsoleMessage(CefRefPtr<CefBrowser> browser, cef_log_severity_t level,
                                        const CefString& message, const CefString& source,
                                        int line) {

    if (auto it = onNewMessage_.find(browser->GetIdentifier()); it != onNewMessage_.end()) {
        it->second(level, message, source, line);
        return true;
    } else if (auto lc = LogCentral::getPtr()) {
        std::string src = source.ToString();

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

        lc->log("WebBrowserClient", loglevel, LogAudience::Developer, src, "", line,
                message.ToString());
        return true;
    }
    return false;
}
cef_return_value_t WebBrowserClient::OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
                                                          CefRefPtr<CefFrame> frame,
                                                          CefRefPtr<CefRequest> request,
                                                          CefRefPtr<CefCallback> callback) {
    CEF_REQUIRE_IO_THREAD();
    return resourceManager_->OnBeforeResourceLoad(browser, frame, request, callback);
}

CefRefPtr<CefResourceHandler> WebBrowserClient::GetResourceHandler(CefRefPtr<CefBrowser> browser,
                                                                   CefRefPtr<CefFrame> frame,
                                                                   CefRefPtr<CefRequest> request) {
    CEF_REQUIRE_IO_THREAD();
    return resourceManager_->GetResourceHandler(browser, frame, request);
}

}  // namespace inviwo
