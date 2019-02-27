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

#ifndef IVW_PROPERTYSYNCEXAMPLEPROCESSOR_H
#define IVW_PROPERTYSYNCEXAMPLEPROCESSOR_H

#include <modules/webbrowser/webbrowsermoduledefine.h>
#include <modules/webbrowser/webbrowserclient.h>
#include <modules/webbrowser/interaction/cefinteractionhandler.h>
#include <modules/webbrowser/cefimageconverter.h>

#include <inviwo/core/interaction/pickingmapper.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/stringproperty.h>

#include <warn/push>
#include <warn/ignore/all>
#include <include/cef_base.h>
#include <warn/pop>

namespace inviwo {

/** \docpage{org.inviwo.PropertySyncExampleProcessor, Property Sync Example Processor}
 * ![](org.inviwo.PropertySyncExampleProcessor.png?classIdentifier=org.inviwo.PropertySyncExampleProcessor)
 * Demonstrates synchronization of properties between browser and Inviwo.
 *
 * ### Inports
 *   * __background__ Background to render web page ontop of.
 * ### Outports
 *   * __webpage__ GUI elements rendered by web browser.
 *
 * ### Properties
 *   * __URL__ Link to webpage, online or file path.
 *   * __Reload__ Fetch page again.
 */

/**
 * \class PropertySyncExampleProcessor
 * \brief Demonstrates synchronization of properties between browser and Inviwo.
 * Synchronization requires two things:
 * 1. Properties must be added to PropertyCefSynchronizer in WebBrowserClient
 * 2. Javascript functions must be added to the html page, see
 * /data/workspaces/web_property_sync.html
 */
class IVW_MODULE_WEBBROWSER_API PropertySyncExampleProcessor : public Processor,
                                                               public CefLoadHandler {
public:
    PropertySyncExampleProcessor();
    virtual ~PropertySyncExampleProcessor();

    virtual void process() override;

    // Detect when page has loaded
    virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack,
                                      bool canGoForward) override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    // Returns "file:/path/modules/webbrowser/data/workspaces/web_property_sync.html"
    std::string getTestWebpageUrl();
    ImageInport background_;
    ImageOutport outport_;

    StringProperty url_;     ///< Web page to show
    ButtonProperty reload_;  ///< Force reload url

    FloatProperty ordinalProp_;
    BoolProperty boolProp_;
    ButtonProperty buttonProp_;
    StringProperty stringProp_;

    CEFInteractionHandler cefInteractionHandler_;
    PickingMapper picking_;
    CefImageConverter cefToInviwoImageConverter_;

    // create browser-window
    CefRefPtr<RenderHandlerGL> renderHandler_;
    CefRefPtr<WebBrowserClient> browserClient_;
    CefRefPtr<CefBrowser> browser_;
    bool isBrowserLoading_ = true;

    IMPLEMENT_REFCOUNTING(PropertySyncExampleProcessor)
};

}  // namespace inviwo

#endif  // IVW_PROPERTYSYNCEXAMPLEPROCESSOR_H
