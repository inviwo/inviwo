/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#include <modules/chromiumembeddedframework/processors/chromiumprocessor.h>
#include <modules/opengl/image/layergl.h>
#include <modules/opengl/texture/texture2d.h>

#include <include/cef_app.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ChromiumProcessor::processorInfo_{
    "org.inviwo.ChromiumProcessor",      // Class identifier
    "Chromium Processor",                // Display name
    "Undefined",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};
const ProcessorInfo ChromiumProcessor::getProcessorInfo() const {
    return processorInfo_;
}

ChromiumProcessor::ChromiumProcessor()
    : Processor() 
    , outport_("image", DataVec4UInt8::get())
    //, doChromiumWork_(100, []() { CefDoMessageLoopWork(); })
    //, renderHandler(new RenderHandler()) 
{
    addPort(outport_);
    
    //{
    //    CefWindowInfo window_info;
    //    CefBrowserSettings browserSettings;

    //    // browserSettings.windowless_frame_rate = 60; // 30 is default

    //    // in linux set a gtk widget, in windows a hwnd. If not available set nullptr - may cause some render errors, in context-menu and plugins.
    //    std::size_t windowHandle = 0;

    //    window_info.SetAsWindowless(nullptr, false); // false means no transparency (site background colour)

    //    browserClient = new BrowserClient(renderHandler);

    //    browser = CefBrowserHost::CreateBrowserSync(window_info, browserClient.get(), "http://www.google.com", browserSettings, nullptr);

    //    // inject user-input by calling - non-trivial for non-windows - checkout the cefclient source and the platform specific cpp, like cefclient_osr_widget_gtk.cpp for linux
    //    // browser->GetHost()->SendKeyEvent(...);
    //    // browser->GetHost()->SendMouseMoveEvent(...);
    //    // browser->GetHost()->SendMouseClickEvent(...);
    //    // browser->GetHost()->SendMouseWheelEvent(...);
    //}
    //doChromiumWork_.start();
}
    
void ChromiumProcessor::process() {
    //outport_.setData(myImage);

}

void RenderHandler::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList &dirtyRects, const void *buffer, int width, int height) {
    auto layerGL = m_renderTexture.getEditableRepresentation<LayerGL>();
    layerGL->getTexture()->upload(buffer);
}

} // namespace

