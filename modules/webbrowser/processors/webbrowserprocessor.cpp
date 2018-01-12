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

#include <include/cef_app.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo WebBrowserProcessor::processorInfo_{
    "org.inviwo.webbrowser",  // Class identifier
    "Web browser",            // Display name
    "Web",                    // Category
    CodeState::Experimental,  // Code state
    Tags::GL,                 // Tags
};
const ProcessorInfo WebBrowserProcessor::getProcessorInfo() const { return processorInfo_; }

WebBrowserProcessor::WebBrowserProcessor()
    : Processor()
    // Output from CEF is 8-bits per channel
    , outport_("webpage", DataVec4UInt8::get())
    , url_("URL", "URL", "http://www.google.com")
    , shader_{"img_convert_cef.frag", true}
    , renderHandler_(new RenderHandlerGL([&]() {
        // Called as soon as new content is available
        // Queue an invalidation
        getNetwork()->getApplication()->dispatchFront(
            [this]() { invalidate(InvalidationLevel::InvalidOutput); });
    }))
    , browserClient_(new WebBrowserClient(renderHandler_)) {

    addPort(outport_);

    {
        CefWindowInfo window_info;

        CefBrowserSettings browserSettings;

        browserSettings.windowless_frame_rate = 30;  // 30 is default

        // in linux set a gtk widget, in windows a hwnd. If not available set nullptr - may cause
        // some render errors, in context-menu and plugins.
        std::size_t windowHandle = 0;
        window_info.SetAsWindowless(
            nullptr);  // nullptr means no transparency (site background colour)

        browser_ = CefBrowserHost::CreateBrowserSync(
            window_info, browserClient_.get(), "http://www.google.com", browserSettings, nullptr);
    }

    addProperty(url_);
    url_.onChange([&]() { browser_->GetMainFrame()->LoadURL(url_.get()); });

    // Inject events into CEF browser_
    addInteractionHandler(new CEFInteractionHandler(browser_->GetHost()));
}

WebBrowserProcessor::~WebBrowserProcessor() {
    // Force close browser
    browser_->GetHost()->CloseBrowser(true);
}

void WebBrowserProcessor::process() {
    // Vertical flip of CEF output image
    utilgl::activateTarget(outport_, ImageType::ColorOnly);
    shader_.activate();

    utilgl::setShaderUniforms(shader_, outport_, "outportParameters_");

    // bind input image
    TextureUnit texUnit;
    utilgl::bindTexture(renderHandler_->getTexture2D(), texUnit);
    shader_.setUniform("inport_", texUnit);

    utilgl::singleDrawImagePlaneRect();
    shader_.deactivate();
    utilgl::deactivateCurrentTarget();
}

void WebBrowserProcessor::invokeEvent(Event* event) {
    Processor::invokeEvent(event);
    switch (event->hash()) {
        case ResizeEvent::chash():
            renderHandler_->updateCanvasSize(outport_.getData()->getDimensions());
            break;
    }
}

}  // namespace inviwo
