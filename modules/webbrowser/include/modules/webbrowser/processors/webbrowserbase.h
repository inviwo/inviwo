/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <modules/webbrowser/webbrowsermoduledefine.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/interaction/pickingmapper.h>
#include <inviwo/core/util/dispatcher.h>
#include <modules/webbrowser/cefimageconverter.h>
#include <modules/webbrowser/interaction/cefinteractionhandler.h>
#include <filesystem>
#include <string_view>

#include <warn/push>
#include <warn/ignore/all>
#include <include/cef_base.h>
#include <include/cef_load_handler.h>
#include <include/wrapper/cef_resource_manager.h>
#include <warn/pop>

class CefBrowser;

namespace inviwo {

class Processor;
class InviwoApplication;
class InteractionHandler;
class RenderHandlerGL;

#include <warn/push>
#include <warn/ignore/dll-interface-base>  // Fine if dependent libs use the same CEF lib binaries
#include <warn/ignore/extra-semi>  // Due to IMPLEMENT_REFCOUNTING, remove when upgrading CEF

/**
 * Base class of a CEF based browser that renders webpages into an ImageOutport.
 * Needs to be wrapped by a CefRefPtr<> due to reference counting.
 *
 * Usage:
 * \code{.cpp}
 *    CefRefPtr<WebBrowserBase> my_class = new WebBrowserBase(app, processor);
 * \endcode
 *
 * See
 * https://bitbucket.org/chromiumembedded/cef/wiki/GeneralUsage#markdown-header-reference-counting
 */
class IVW_MODULE_WEBBROWSER_API WebBrowserBase : public CefLoadHandler {
public:
    WebBrowserBase(InviwoApplication* app, Processor* processor, std::string_view url = "",
                   std::function<void()> onNewRender = nullptr,
                   std::function<void(bool)> onLoadingChanged = nullptr);
    WebBrowserBase(const WebBrowserBase& rhs) = delete;
    WebBrowserBase(WebBrowserBase&& rhs) = delete;
    WebBrowserBase& operator=(const WebBrowserBase& rhs) = delete;
    WebBrowserBase& operator=(WebBrowserBase&& rhs) = delete;
    ~WebBrowserBase();

    void reload();
    bool isLoading() const;

    void load(const std::filesystem::path& filename);
    void load(std::string_view url);
    const std::string& getURL() const;

    /**
     * Clear the browser frame and load an empty page
     */
    void clear();

    /**
     * Set the zoom level of the browser to the absolute @p zoom factor.
     * @param zoom   absolute zoom factor, i.e. 1.0 corresponds to a scaling of 100%
     */
    void setZoom(double zoom);

    void render(ImageOutport& outport, ImageInport* background = nullptr);

    void executeJavaScript(const std::string& javascript, int startLine = 1);

    InteractionHandler* getInteractionHandler();

    /**
     * Registers a callback function @p f that will be called whenever the web page has been loaded.
     * @param f   callback function
     * @return RAII callback
     * \see CefLoadHandler::OnLoadingStateChange
     */
    [[nodiscard]] std::shared_ptr<BaseCallBack> addLoadingDoneCallback(std::function<void()> f);

    void addMessageHandler(
        std::function<void(cef_log_severity_t, const CefString&, const CefString&, int)> func);

    void addStaticHandler(
        std::function<bool(const std::string&, scoped_refptr<CefResourceManager::Request>)>
            handler);
    void removeStaticHandler();

    std::shared_ptr<std::function<std::string(const std::string&)>> registerCallback(
        const std::string& name, std::function<std::string(const std::string&)> callback);

private:
    virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack,
                                      bool canGoForward) override;

    Processor* parentProcessor_;

    PickingMapper picking_;

    CEFInteractionHandler cefInteractionHandler_;
    CefImageConverter cefToInviwoImageConverter_;
    // create browser-window
    CefRefPtr<RenderHandlerGL> renderHandler_;
    CefRefPtr<CefBrowser> browser_;

    bool isLoading_ = true;
    std::string url_;

    std::function<void(bool)> onLoadingChanged_;
    Dispatcher<void()> loadingDone_;

    IMPLEMENT_REFCOUNTING(WebBrowserBase);
};
#include <warn/pop>

}  // namespace inviwo
