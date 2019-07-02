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

#include <modules/webbrowser/processors/webbrowserprocessor.h>
#include <modules/webbrowser/interaction/cefinteractionhandler.h>
#include <modules/webbrowser/webbrowsermodule.h>
#include <modules/opengl/image/layergl.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/propertyfactory.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/utilities.h>

#include <warn/push>
#include <warn/ignore/all>
#include <include/cef_app.h>
#include <warn/pop>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo WebBrowserProcessor::processorInfo_{
    "org.inviwo.webbrowser",  // Class identifier
    "Web browser",            // Display name
    "Web",                    // Category
    CodeState::Stable,        // Code state
    "GL, Web Browser",        // Tags
};
const ProcessorInfo WebBrowserProcessor::getProcessorInfo() const { return processorInfo_; }

WebBrowserProcessor::WebBrowserProcessor()
    : Processor()
    // Output from CEF is 8-bits per channel
    , background_("background")
    , outport_("webpage", DataVec4UInt8::get())
    , fileName_("fileName", "HTML file", "")
    , autoReloadFile_("autoReloadFile", "Auto Reload", true)
    , url_("URL", "URL", "http://www.inviwo.org")
    , reload_("reload", "Reload")
    , runJS_("runJS", "Run JS")
    , js_("js", "JavaScript", "", InvalidationLevel::Valid)
    , sourceType_("sourceType", "Source",
                  {{"localFile", "Local File", SourceType::LocalFile},
                   {"webAddress", "Web Address", SourceType::WebAddress}})
    , picking_(this, 1, [&](PickingEvent* p) { cefInteractionHandler_.handlePickingEvent(p); })
    , cefToInviwoImageConverter_(picking_.getColor())
    , renderHandler_(new RenderHandlerGL([&]() {
        // Called as soon as new content is available
        // Queue an invalidation
        // Note: no need to queue invalidation using dispathFront since
        // RenderHandler calls will be made from the same thread.
        // dispatchFront([&] {
        invalidate(InvalidationLevel::InvalidOutput);
        //});
    }))
    , browserClient_(new WebBrowserClient(renderHandler_, InviwoApplication::getPtr()
                                                              ->getModuleByType<WebBrowserModule>()
                                                              ->getPropertyWidgetCEFFactory())) {
    addPort(background_);
    background_.setOptional(true);
    addPort(outport_);

    addProperty(sourceType_);
    addProperty(fileName_);
    addProperty(autoReloadFile_);
    addProperty(url_);
    url_.setVisible(false);
    addProperty(reload_);

    addProperty(runJS_);
    addProperty(js_);

    auto updateVisibility = [this]() {
        fileName_.setVisible(sourceType_ == SourceType::LocalFile);
        autoReloadFile_.setVisible(sourceType_ == SourceType::LocalFile);
        url_.setVisible(sourceType_ == SourceType::WebAddress);
    };
    updateVisibility();

    auto reload = [this]() { browser_->GetMainFrame()->LoadURL(getSource()); };

    sourceType_.onChange([reload, updateVisibility]() {
        updateVisibility();
        reload();
    });
    fileName_.onChange([this, reload]() {
        if (autoReloadFile_) {
            fileObserver_.setFilename(fileName_);
        }
        reload();
    });
    autoReloadFile_.onChange([this]() {
        if (autoReloadFile_) {
            fileObserver_.setFilename(fileName_);
        } else {
            fileObserver_.stop();
        }
    });
    url_.onChange(reload);
    reload_.onChange(reload);

    fileObserver_.onChange([this, reload]() {
        if (sourceType_ == SourceType::LocalFile) {
            reload();
        }
    });

    // Setup CEF browser
    CefWindowInfo window_info;

    CefBrowserSettings browserSettings;

    // Enable loading files from other locations than where the .html file is
    browserSettings.file_access_from_file_urls = STATE_ENABLED;

    browserSettings.windowless_frame_rate = 30;  // Must be between 1-60, 30 is default

    // in linux set a gtk widget, in windows a hwnd. If not available set nullptr
    // - may cause
    // some render errors, in context-menu and plugins.
    window_info.SetAsWindowless(nullptr);  // nullptr means no transparency (site background colour)

    // Note that browserClient_ outlives this class so make sure to remove
    // renderHandler_ in
    // destructor
    browser_ = CefBrowserHost::CreateBrowserSync(window_info, browserClient_, getSource(),
                                                 browserSettings, nullptr);
    // Observe when page has loaded
    browserClient_->addLoadHandler(this);
    // Do not process until frame is loaded
    isReady_.setUpdate([this]() { return allInportsAreReady() && !isBrowserLoading_; });
    // Inject events into CEF browser_
    cefInteractionHandler_.setHost(browser_->GetHost());
    cefInteractionHandler_.setRenderHandler(renderHandler_);
    addInteractionHandler(&cefInteractionHandler_);
}

std::string WebBrowserProcessor::getSource() {
    std::string sourceString;
    if (sourceType_.get() == SourceType::LocalFile) {
        sourceString = "file://" + fileName_.get();
    } else if (sourceType_.get() == SourceType::WebAddress) {
        sourceString = url_.get();
    }
#ifndef NDEBUG
    // CEF does not allow empty urls in debug mode
    if (sourceString.empty()) {
        sourceString = "https://www.inviwo.org";
    }
#endif
    return sourceString;
}

WebBrowserProcessor::~WebBrowserProcessor() {
    browserClient_->removeLoadHandler(this);
    // Force close browser
    browser_->GetHost()->CloseBrowser(true);
    // Remove render handler since browserClient_ might not be destroyed until CefShutdown() is
    // called
    browserClient_->SetRenderHandler(NULL);
}

void WebBrowserProcessor::deserialize(Deserializer& d) {
    Processor::deserialize(d);
    // Must reload page to connect property with Frame, see PropertyCefSynchronizer::OnLoadEnd
    browser_->GetMainFrame()->LoadURL(getSource());
}

void WebBrowserProcessor::OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading,
                                               bool /*canGoBack*/, bool /*canGoForward*/) {
    if (browser_ && browser->GetIdentifier() == browser_->GetIdentifier()) {
        isBrowserLoading_ = isLoading;
        isReady_.update();
    }
}

void WebBrowserProcessor::process() {
    if (js_.isModified() && !js_.get().empty()) {
        browser_->GetMainFrame()->ExecuteJavaScript(js_.get(), "", 1);
    }

    // Vertical flip of CEF output image
    cefToInviwoImageConverter_.convert(renderHandler_->getTexture2D(), outport_, &background_);
}

}  // namespace inviwo
