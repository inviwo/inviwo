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

#include <modules/webbrowser/processors/webbrowserbase.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/moduleutils.h>

#include <inviwo/core/processors/processor.h>
#include <modules/webbrowser/webbrowserclient.h>
#include <modules/webbrowser/webbrowsermodule.h>
#include <modules/webbrowser/webbrowserutil.h>

#include <fmt/core.h>

#include <utility>

namespace inviwo {

WebBrowserBase::WebBrowserBase(InviwoApplication* app, Processor* processor, const std::string& url)
    : parentProcessor_{processor}
    , picking_{processor, 1,
               [this](PickingEvent* p) { cefInteractionHandler_.handlePickingEvent(p); }}
    , cefToInviwoImageConverter_{picking_.getColor()}
    , renderHandler_{dynamic_cast<RenderHandlerGL*>(
          app->getModuleByType<WebBrowserModule>()->getBrowserClient()->GetRenderHandler().get())}
    , url_{url} {

    // Setup CEF browser
    auto [windowInfo, browserSettings] = cefutil::getDefaultBrowserSettings();
    auto browserClient = app->getModuleByType<WebBrowserModule>()->getBrowserClient();
    // Note that browserClient_ outlives this class so make sure to remove
    // this CefLoadHandler in destructor
    browser_ = CefBrowserHost::CreateBrowserSync(windowInfo, browserClient, getURL(),
                                                 browserSettings, nullptr, nullptr);
    browserClient->setBrowserParent(browser_, parentProcessor_);
    // Observe when page has loaded
    browserClient->addLoadHandler(this);
    // Inject events into CEF browser
    cefInteractionHandler_.setHost(browser_->GetHost());
    cefInteractionHandler_.setRenderHandler(renderHandler_.get());
}

WebBrowserBase::~WebBrowserBase() {
    auto* client = dynamic_cast<WebBrowserClient*>(browser_->GetHost()->GetClient().get());
    client->removeLoadHandler(this);
    // Explicitly remove parent because CloseBrowser may result in WebBrowserClient::OnBeforeClose
    // after this processor has been destroyed.
    client->removeBrowserParent(browser_);
    // Force close browser
    browser_->GetHost()->CloseBrowser(true);
}

void WebBrowserBase::render(ImageOutport& outport, ImageInport* background) {
    if (isLoading_) {
        return;
    }

    // Vertical flip of CEF output image
    cefToInviwoImageConverter_.convert(renderHandler_->getTexture2D(browser_), outport, background);
}

void WebBrowserBase::executeJavaScript(const std::string& javascript, int startLine) {
    browser_->GetMainFrame()->ExecuteJavaScript(javascript, "WebBrowserBase", startLine);
}

void WebBrowserBase::reload() {
    isLoading_ = true;
    browser_->GetMainFrame()->LoadURL(getURL());
}

bool WebBrowserBase::isLoading() const { return isLoading_; }

void WebBrowserBase::load(const std::filesystem::path& filename) {
    if (!filename.empty()) {
        url_ = fmt::format("file://{}", filename.string());
        reload();
    } else if (!url_.empty()) {
        url_.clear();
        reload();
    }
}

void WebBrowserBase::load(std::string_view url) {
    if (url_ != url) {
        url_ = url;
        reload();
    }
}

void WebBrowserBase::clear() {
    url_ = fmt::format(
        "file://{}",
        (util::getModulePath("WebBrowser", ModulePath::Data) / "html/empty.html").string());
    reload();
}

void WebBrowserBase::setZoom(double zoom) {
    browser_->GetHost()->SetZoomLevel(cefutil::percentageToZoomLevel(zoom));
}

std::shared_ptr<BaseCallBack> WebBrowserBase::addLoadingDoneCallback(std::function<void()> f) {
    return loadingDone_.add(std::move(f));
}

const std::string& WebBrowserBase::getURL() const {
#ifndef NDEBUG
    // CEF does not allow empty urls in debug mode
    if (url_.empty()) {
        return fmt::format(
            "file://{}",
            (util::getModulePath("WebBrowser", ModulePath::Data) / "html/empty.html").string());
    }
#endif
    return url_;
}

void WebBrowserBase::OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading,
                                          bool /*canGoBack*/, bool /*canGoForward*/) {
    if (browser_ && browser->GetIdentifier() == browser_->GetIdentifier()) {
        isLoading_ = isLoading;
        // Render new page (content may have been rendered before the state changed)
        if (!isLoading && parentProcessor_) {
            loadingDone_.invoke();
            parentProcessor_->invalidate(InvalidationLevel::InvalidOutput);
        }
    }
}

InteractionHandler* WebBrowserBase::getInteractionHandler() { return &cefInteractionHandler_; }

}  // namespace inviwo
