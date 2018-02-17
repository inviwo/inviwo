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

#ifndef IVW_WEBBROWSERPROCESSOR_H
#define IVW_WEBBROWSERPROCESSOR_H

#include <modules/webbrowser/webbrowsermoduledefine.h>
#include <modules/webbrowser/webbrowserclient.h>
#include <modules/webbrowser/interaction/cefinteractionhandler.h>

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/properties/stringproperty.h>

#include <modules/opengl/shader/shader.h>

#include <warn/push>
#include <warn/ignore/all>
#include <include/cef_base.h>
#include <warn/pop>

namespace inviwo {
class WebBrowser;

/** \docpage{org.inviwo.WebBrowser, Chromium Processor}
 * ![](org.inviwo.WebBrowser.png?classIdentifier=org.inviwo.WebBrowser)
 * Renders webpage, including transparency, into output image color layer. 
 * Forwards events to webpage but does not consume them.
 *
 * ### Outports
 *   * __webpage__ Rendered web page.
 *
 * ### Properties
 *   * __URL__ Link to webpage, online or file path.
 */
/**
 * \class WebBrowser
 * \brief Render webpage into the color layer (OpenGL).
 */
class IVW_MODULE_WEBBROWSER_API WebBrowserProcessor : public Processor {
public:
    WebBrowserProcessor();
    virtual ~WebBrowserProcessor();

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    // Forward resize events to renderHandler_
    virtual void invokeEvent(Event *event) override; 

private:
    ImageOutport outport_;

    StringProperty url_; ///< Web page to show

    Shader shader_;  ///< Flip image y compoenent 

    // create browser-window
    CefRefPtr<RenderHandlerGL> renderHandler_;
    CefRefPtr<WebBrowserClient> browserClient_;
    CefRefPtr<CefBrowser> browser_;

    CEFInteractionHandler cefInteractionHandler_;
};

}  // namespace inviwo

#endif  // IVW_WEBBROWSERPROCESSOR_H
