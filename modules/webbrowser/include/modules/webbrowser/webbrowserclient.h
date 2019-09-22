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

#ifndef IVW_WEBBROWSERCLIENT_H
#define IVW_WEBBROWSERCLIENT_H

#include <modules/webbrowser/webbrowsermoduledefine.h>
#include <modules/webbrowser/renderhandlergl.h>
#include <modules/webbrowser/properties/propertycefsynchronizer.h>
#include <modules/webbrowser/processors/processorcefsynchronizer.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/stdextensions.h>

#include <warn/push>
#include <warn/ignore/all>
#include <include/cef_client.h>
#include <include/cef_load_handler.h>
#include <include/cef_life_span_handler.h>
#include "include/wrapper/cef_message_router.h"
#include "include/wrapper/cef_resource_manager.h"
#include <warn/pop>

namespace inviwo {

/* \class WebBrowserClient
 * CefClient with custom render handler and call redirections.
 * Calls to 'https://inviwo/modules/yourmodule' will be redirected to yourmodule
 * directory, i.e. InviwoModule::getPath().
 * Calls to 'https://inviwo/app' will be redirected to the InviwoApplication
 * (executable) directory, i.e. InviwoApplication::getBasePath().
 */
#include <warn/push>
#include <warn/ignore/dll-interface-base>  // Fine if dependent libs use the same CEF lib binaries
#include <warn/ignore/extra-semi>  // Due to IMPLEMENT_REFCOUNTING, remove when upgrading CEF
class IVW_MODULE_WEBBROWSER_API WebBrowserClient : public CefClient,
                                                   public CefLifeSpanHandler,
                                                   public CefRequestHandler,
                                                   public CefLoadHandler,
                                                   public CefDisplayHandler {
public:
    WebBrowserClient(CefRefPtr<RenderHandlerGL> renderHandler,
                     const PropertyWidgetCEFFactory* widgetFactory);

    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }
    virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override { return renderHandler_; }
    virtual CefRefPtr<CefRequestHandler> GetRequestHandler() override { return this; }
    virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }

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

    cef_return_value_t OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,
                                            CefRefPtr<CefFrame> frame,
                                            CefRefPtr<CefRequest> request,
                                            CefRefPtr<CefRequestCallback> callback) override;

    CefRefPtr<CefResourceHandler> GetResourceHandler(CefRefPtr<CefBrowser> browser,
                                                     CefRefPtr<CefFrame> frame,
                                                     CefRefPtr<CefRequest> request) override;
    // CefLoadHandler methods:
    /*
     * Added handlers will receive CefLoadHandler calls.
     * @see OnLoadingStateChange
     * @see OnLoadEnd
     * @see OnLoadError
     * @see removeLoadHandler
     */
    void addLoadHandler(CefLoadHandler* loadHandler);
    void removeLoadHandler(CefLoadHandler* loadHandler);
    ///
    // Called when the loading state has changed. This callback will be executed
    // twice -- once when loading is initiated either programmatically or by user
    // action, and once when loading is terminated due to completion, cancellation
    // of failure. It will be called before any calls to OnLoadStart and after all
    // calls to OnLoadError and/or OnLoadEnd.
    ///
    /*--cef()--*/
    virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack,
                                      bool canGoForward) override;
    /**
     * Synchronizes all widgets and sets their frame, called when frame has loaded.
     */
    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                           int httpStatusCode) override;
    ///
    // Inviwo: First forwards the call to addded load handlers and then
    // displays an error page in the given frame.
    // Called when a navigation fails or is canceled. This method may be called
    // by itself if before commit or in combination with OnLoadStart/OnLoadEnd if
    // after commit. |errorCode| is the error code number, |errorText| is the
    // error text and |failedUrl| is the URL that failed to load.
    // See net\base\net_error_list.h for complete descriptions of the error codes.
    //
    ///
    /*--cef(optional_param=errorText)--*/
    virtual void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                             CefLoadHandler::ErrorCode errorCode, const CefString& errorText,
                             const CefString& failedUrl) override;

    CefRefPtr<PropertyCefSynchronizer> propertyCefSynchronizer_;

    ///
    // Inviwo: Overload to log console.log message from js to inviwo the inviwo::LogCentral
    // Cef: Called to display a console message. Return true to stop the message from
    // being output to the console.
    ///
    /*--cef(optional_param=message,optional_param=source)--*/
    virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser, cef_log_severity_t level,
                                  const CefString& message, const CefString& source,
                                  int line) override;

protected:
    const PropertyWidgetCEFFactory* widgetFactory_;  /// Non-owning reference
    CefRefPtr<CefRenderHandler> renderHandler_;
    // Handles the browser side of query routing.
    CefRefPtr<CefMessageRouterBrowserSide> messageRouter_;

    CefRefPtr<ProcessorCefSynchronizer> processorCefSynchronizer_;

    std::vector<CefLoadHandler*> loadHandlers_;
    // Manages the registration and delivery of resources (redirections to
    // modules/app folders).
    CefRefPtr<CefResourceManager> resourceManager_;

    // Track the number of browsers using this Client.
    int browserCount_ = 0;

private:
    IMPLEMENT_REFCOUNTING(WebBrowserClient);
    DISALLOW_COPY_AND_ASSIGN(WebBrowserClient);
};
#include <warn/pop>
}  // namespace inviwo

#endif  // IVW_WEBBROWSERCLIENT_H
