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

#include <modules/webbrowser/processors/webbrowserprocessor.h>
#include <modules/webbrowser/interaction/cefinteractionhandler.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/shader/shaderutils.h>

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
    , background_("background")
    // Output from CEF is 8-bits per channel
    , outport_("webpage", DataVec4UInt8::get())
    , url_("URL", "URL", "http://www.google.com")
    , reload_("reload", "Reload")
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

    {
        CefWindowInfo window_info;

        CefBrowserSettings browserSettings;

        browserSettings.windowless_frame_rate = 30;  // Must be between 1-60, 30 is default

        // in linux set a gtk widget, in windows a hwnd. If not available set nullptr - may cause
        // some render errors, in context-menu and plugins.
        window_info.SetAsWindowless(
            nullptr);  // nullptr means no transparency (site background colour)

        // Note that browserClient_ outlives this class so make sure to remove renderHandler_ in
        // destructor
        browser_ = CefBrowserHost::CreateBrowserSync(window_info, browserClient_, url_.get(),
                                                     browserSettings, nullptr);
    }

    addProperty(url_);
    addProperty(reload_);
    // Inject events into CEF browser_
    cefInteractionHandler_.setHost(browser_->GetHost());
    cefInteractionHandler_.setRenderHandler(renderHandler_);
    addInteractionHandler(&cefInteractionHandler_);
}

WebBrowserProcessor::~WebBrowserProcessor() {
    // Force close browser
    browser_->GetHost()->CloseBrowser(true);
    // Remove render handler since browserClient_ might not be destroyed until CefShutdown() is
    // called
    browserClient_->SetRenderHandler(NULL);
}

void WebBrowserProcessor::process() {
    if (url_.isModified() || reload_.isModified()) {
        browser_->GetMainFrame()->LoadURL(url_.get());
    }

    // Vertical flip of CEF output image
    cefToInviwoImageConverter_.convert(renderHandler_->getTexture2D(), outport_, &background_);
}

}  // namespace inviwo
