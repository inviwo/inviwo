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

#include <modules/webbrowser/processors/propertysyncexampleprocessor.h>
#include <modules/webbrowser/interaction/cefinteractionhandler.h>
#include <modules/webbrowser/webbrowsermodule.h>
#include <modules/opengl/image/layergl.h>
#include <inviwo/core/util/filesystem.h>

#include <warn/push>
#include <warn/ignore/all>
#include <include/cef_app.h>
#include <warn/pop>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo PropertySyncExampleProcessor::processorInfo_{
    "org.inviwo.PropertySyncExampleProcessor",  // Class identifier
    "Property Sync Example",                    // Display name
    "Webbrowser",                               // Category
    CodeState::Experimental,                    // Code state
    "GL, Web Browser, Test",                    // Tags
};
const ProcessorInfo PropertySyncExampleProcessor::getProcessorInfo() const {
    return processorInfo_;
}

PropertySyncExampleProcessor::PropertySyncExampleProcessor()
    : Processor()
    // Output from CEF is 8-bits per channel
    , background_("background")
    , outport_("webpage", DataVec4UInt8::get())
    , url_("URL", "URL", getTestWebpageUrl())
    , reload_("reload", "Reload")
    // Properties, note that InvalidationLevel::Valid is used to not get
    // invalidations from both CEF (html) and Qt
    , ordinalProp_("ordinalProperty", "Ordinal", 1.f, 0.f, 2.f, 0.1f, InvalidationLevel::Valid)
    , boolProp_("boolProperty", "Bool", true, InvalidationLevel::Valid)
    , buttonProp_("buttonProperty", "Button", InvalidationLevel::Valid)
    , stringProp_("stringProperty", "String", "Edit me", InvalidationLevel::Valid)
    , picking_(this, 1, [&](PickingEvent* p) { cefInteractionHandler_.handlePickingEvent(p); })
    , cefToInviwoImageConverter_(picking_.getColor())
    , renderHandler_(new RenderHandlerGL([&]() {
        // Called as soon as new content is available
        // Queue an invalidation
        // Note: no need to queue invalidation using dispathFront since
        // RenderHandler calls will be made from the same thread.
        invalidate(InvalidationLevel::InvalidOutput);
    }))
    , browserClient_(new WebBrowserClient(renderHandler_)) {
    addPort(background_);
    background_.setOptional(true);
    addPort(outport_);

    CefWindowInfo window_info;

    CefBrowserSettings browserSettings;

    browserSettings.windowless_frame_rate = 30;  // Must be between 1-60, 30 is default

    // in linux set a gtk widget, in windows a hwnd. If not available set nullptr - may cause
    // some render errors, in context-menu and plugins.
    window_info.SetAsWindowless(nullptr);  // nullptr means no transparency (site background colour)

    // Observe when page has loaded
    browserClient_->addLoadHandler(this);
    // Do not process until frame is loaded
    isReady_.setUpdate([this]() { return allInportsAreReady() && !isBrowserLoading_; });

    // Note that browserClient_ outlives this class so make sure to remove renderHandler_ in
    // destructor
    auto url = getTestWebpageUrl();
    browser_ = CefBrowserHost::CreateBrowserSync(window_info, browserClient_, url, browserSettings,
                                                 nullptr);
    browser_->GetMainFrame()->LoadURL(url);
    url_.setReadOnly(true);
    addProperty(url_);
    addProperty(reload_);
    url_.onChange([this]() { browser_->GetMainFrame()->LoadURL(url_.get()); });
    reload_.onChange([this]() { browser_->GetMainFrame()->LoadURL(url_.get()); });
    // Inject events into CEF browser_
    cefInteractionHandler_.setHost(browser_->GetHost());
    cefInteractionHandler_.setRenderHandler(renderHandler_);
    addInteractionHandler(&cefInteractionHandler_);
    // Add property to processor before propertyCefSynchronizer to
    // include processor in property path
    addProperty(ordinalProp_);
    addProperty(boolProp_);
    addProperty(buttonProp_);
    addProperty(stringProp_);

    browserClient_->propertyCefSynchronizer_->startSynchronize(&ordinalProp_);
    browserClient_->propertyCefSynchronizer_->startSynchronize(&boolProp_);
    browserClient_->propertyCefSynchronizer_->startSynchronize(&buttonProp_);
    browserClient_->propertyCefSynchronizer_->startSynchronize(&stringProp_);
}

PropertySyncExampleProcessor::~PropertySyncExampleProcessor() {
    browserClient_->removeLoadHandler(this);
    // Force close browser
    browser_->GetHost()->CloseBrowser(true);
    // Remove render handler since browserClient_ might not be destroyed until CefShutdown() is
    // called
    browserClient_->SetRenderHandler(NULL);
}

void PropertySyncExampleProcessor::process() {
    // Vertical flip of CEF output image
    cefToInviwoImageConverter_.convert(renderHandler_->getTexture2D(), outport_, &background_);
}

std::string PropertySyncExampleProcessor::getTestWebpageUrl() {
    // Cannot use this->getInviwoApplication in processor since it has not been set upon
    // construction.
    auto app = InviwoApplication::getPtr();
    auto module = app->getModuleByType<WebBrowserModule>();
    auto path = module->getPath(ModulePath::Workspaces) + "/web_property_sync.html";
    if (!filesystem::fileExists(path)) {
        throw Exception("Could not find " + path);
    }
    return "file://" + path;
}

void PropertySyncExampleProcessor::OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
                                                        bool isLoading, bool /*canGoBack*/,
                                                        bool /*canGoForward*/) {
    if (browser_ && browser->GetIdentifier() == browser_->GetIdentifier()) {
        isBrowserLoading_ = isLoading;
        isReady_.update();
    }
}

}  // namespace inviwo
