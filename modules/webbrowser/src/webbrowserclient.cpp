/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2023 Inviwo Foundation
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

#include <inviwo/core/common/inviwoapplication.h>                    // for InviwoApplication
#include <inviwo/core/common/inviwomodule.h>                         // for InviwoModule
#include <inviwo/core/common/modulemanager.h>                        // for ModuleManager
#include <inviwo/core/processors/processor.h>                        // for Processor
#include <inviwo/core/properties/invalidationlevel.h>                // for InvalidationLevel
#include <inviwo/core/util/exception.h>                              // for Exception
#include <inviwo/core/util/logcentral.h>                             // for LogCentral, LogError
#include <inviwo/core/util/sourcecontext.h>                          // for IVW_CONTEXT
#include <inviwo/core/util/stdextensions.h>                          // for erase_remove
#include <inviwo/core/util/stringconversion.h>                       // for replaceInString, spl...
#include <modules/webbrowser/processors/processorcefsynchronizer.h>  // for ProcessorCefSynchron...
#include <modules/webbrowser/properties/propertycefsynchronizer.h>   // for PropertyCefSynchronizer
#include <modules/webbrowser/renderhandlergl.h>                      // for RenderHandlerGL, Ren...
#include <modules/webbrowser/webbrowsermodule.h>                     // for CefString, cef_log_s...

#include <ostream>      // for operator<<, basic_os...
#include <string_view>  // for string_view
#include <utility>      // for pair

#include <warn/push>
#include <warn/ignore/all>
#include <fmt/core.h>                              // for format
#include <include/base/cef_bind.h>                 // for Bind
#include <include/base/cef_logging.h>              // for COMPACT_GOOGLE_LOG_D...
#include <include/base/cef_scoped_refptr.h>        // for scoped_refptr
#include <include/cef_base.h>                      // for CefRefPtr
#include <include/cef_browser.h>                   // for CefBrowser
#include <include/cef_frame.h>                     // for CefFrame
#include <include/cef_life_span_handler.h>         // for CefLifeSpanHandler
#include <include/cef_load_handler.h>              // for CefLoadHandler, CefL...
#include <include/cef_process_message.h>           // for CefProcessMessage
#include <include/cef_request.h>                   // for CefRequest
#include <include/cef_request_handler.h>           // for CefRequestHandler::T...
#include <include/cef_resource_request_handler.h>  // for CefResourceRequestHa...
#include <include/cef_task.h>                      // for CefCurrentlyOn
#include <include/wrapper/cef_closure_task.h>      // for CefPostTask
#include <include/wrapper/cef_helpers.h>           // for CEF_REQUIRE_UI_THREAD
#include <include/wrapper/cef_message_router.h>    // for CefMessageRouterBrow...
#include <include/wrapper/cef_resource_manager.h>  // for CefResourceManager

class CefResourceHandler;

namespace inviwo {
class PropertyWidgetCEFFactory;
}  // namespace inviwo

#include <warn/pop>

namespace inviwo {

namespace detail {

// Register inviwo application and module directories for resource loading
void setupResourceManager(CefRefPtr<CefResourceManager> resource_manager) {
    if (!CefCurrentlyOn(TID_IO)) {
        // Execute on the browser IO thread.
        CefPostTask(TID_IO, base::BindRepeating(setupResourceManager, resource_manager));
        return;
    }
    std::string origin = "inviwo://";
    // Redirect paths to corresponding app/module directories.
    // Enables resource loading from these directories directory (js-files and so on).
    auto appOrigin = origin + "app";
    resource_manager->AddDirectoryProvider(
        appOrigin, InviwoApplication::getPtr()->getBasePath().string(), 99, std::string());

    auto moduleOrigin = origin;

    for (const auto& m : InviwoApplication::getPtr()->getModuleManager().getInviwoModules()) {
        auto mOrigin = moduleOrigin + toLower(m.getIdentifier());
        auto moduleDir = m.getPath();
        resource_manager->AddDirectoryProvider(mOrigin, moduleDir.string(), 100, std::string());
    }
}

}  // namespace detail

WebBrowserClient::WebBrowserClient(ModuleManager& moduleManager,
                                   const PropertyWidgetCEFFactory* widgetFactory)
    : widgetFactory_{widgetFactory}
    , renderHandler_(new RenderHandlerGL([&](CefRefPtr<CefBrowser> browser) {
        auto bdIt = browserParents_.find(browser->GetIdentifier());
        if (bdIt != browserParents_.end()) {
            bdIt->second.processor->invalidate(InvalidationLevel::InvalidOutput);
        }
    }))
    , resourceManager_(new CefResourceManager()) {
    onModulesRegisteredCallback_ = moduleManager.onModulesDidRegister([&]() {
        // Ensure that all module resources have been registered before setting up resources
        detail::setupResourceManager(resourceManager_);
    });
}

void WebBrowserClient::setBrowserParent(CefRefPtr<CefBrowser> browser, Processor* parent) {
    CEF_REQUIRE_UI_THREAD();
    BrowserData bd{parent, new ProcessorCefSynchronizer(parent)};
    browserParents_[browser->GetIdentifier()] = bd;
    addLoadHandler(bd.processorCefSynchronizer.get());
    messageRouter_->AddHandler(bd.processorCefSynchronizer.get(), false);
}

void WebBrowserClient::removeBrowserParent(CefRefPtr<CefBrowser> browser) {
    auto bdIt = browserParents_.find(browser->GetIdentifier());
    if (bdIt != browserParents_.end()) {
        messageRouter_->RemoveHandler(bdIt->second.processorCefSynchronizer.get());
        removeLoadHandler(bdIt->second.processorCefSynchronizer.get());
        browserParents_.erase(bdIt);
    }
}

ProcessorCefSynchronizer::CallbackHandle WebBrowserClient::registerCallback(
    CefRefPtr<CefBrowser> browser, const std::string& name,
    std::function<ProcessorCefSynchronizer::CallbackFunc> callback) {
    CEF_REQUIRE_UI_THREAD();
    if (auto it = browserParents_.find(browser->GetIdentifier()); it != browserParents_.end()) {
        return it->second.processorCefSynchronizer->registerCallback(name, callback);
    } else {
        throw Exception(
            fmt::format(
                "Registering callback '{}' in browser without a parent processor (browser ID {})",
                name, browser->GetIdentifier()),
            IVW_CONTEXT);
    }
}

bool WebBrowserClient::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                                CefRefPtr<CefFrame> frame,
                                                CefProcessId source_process,
                                                CefRefPtr<CefProcessMessage> message) {
    CEF_REQUIRE_UI_THREAD();
    return messageRouter_->OnProcessMessageReceived(browser, frame, source_process, message);
}

void WebBrowserClient::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
    CEF_REQUIRE_UI_THREAD();

    if (!messageRouter_) {
        // Create the browser-side router for query handling.
        CefMessageRouterConfig config;
        messageRouter_ = CefMessageRouterBrowserSide::Create(config);
    }
    // Create a Property synchronizer for the browser
    propertyCefSynchronizers_[browser->GetIdentifier()] =
        std::make_unique<PropertyCefSynchronizer>(browser, widgetFactory_);
    auto synchronizer = propertyCefSynchronizers_[browser->GetIdentifier()].get();
    addLoadHandler(synchronizer);
    messageRouter_->AddHandler(synchronizer, false);

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
    removeBrowserParent(browser);
    // Remove associated Property synchronizer
    messageRouter_->RemoveHandler(propertyCefSynchronizers_[browser->GetIdentifier()].get());
    removeLoadHandler(propertyCefSynchronizers_[browser->GetIdentifier()].get());
    propertyCefSynchronizers_.erase(browser->GetIdentifier());
    if (--browserCount_ == 0) {
        // Free the router when the last browser is closed.
        messageRouter_.reset();
    }

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
            file = std::string{util::splitByLast(src, '/').second};
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
