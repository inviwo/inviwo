/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2023 Inviwo Foundation
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

#include <modules/webbrowser/webbrowsermoduledefine.h>  // for IVW_MODULE_WEBBROWSER_API

#include <inviwo/core/interaction/pickingmapper.h>                 // for PickingMapper
#include <inviwo/core/ports/imageport.h>                           // for ImageInport, ImageOutport
#include <inviwo/core/processors/processor.h>                      // for Processor
#include <inviwo/core/processors/processorinfo.h>                  // for ProcessorInfo
#include <inviwo/core/properties/boolproperty.h>                   // for BoolProperty
#include <inviwo/core/properties/buttonproperty.h>                 // for ButtonProperty
#include <inviwo/core/properties/fileproperty.h>                   // for FileProperty
#include <inviwo/core/properties/optionproperty.h>                 // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>                // for DoubleProperty
#include <inviwo/core/properties/stringproperty.h>                 // for StringProperty
#include <inviwo/core/util/singlefileobserver.h>                   // for SingleFileObserver
#include <inviwo/core/util/staticstring.h>                         // for operator+
#include <modules/webbrowser/cefimageconverter.h>                  // for CefImageConverter
#include <modules/webbrowser/interaction/cefinteractionhandler.h>  // for CEFInteractionHandler

#include <functional>   // for __base
#include <string>       // for operator==, operator+
#include <string_view>  // for operator==
#include <vector>       // for operator!=, vector

#include <warn/push>
#include <warn/ignore/all>
#include <include/cef_base.h>          // for CefRefPtr, IMPLEMENT_R...
#include <include/cef_load_handler.h>  // for CefLoadHandler

class CefBrowser;
namespace inviwo {
class Deserializer;
class InviwoApplication;
class RenderHandlerGL;
}  // namespace inviwo

#include <warn/pop>

namespace inviwo {

/** \docpage{org.inviwo.WebBrowser, Chromium Processor}
 * ![](org.inviwo.WebBrowser.png?classIdentifier=org.inviwo.WebBrowser)
 * Display webpage, including transparency, on top of optional background and enable synchronization
 * of properties.
 *
 * Synchronization from Invwo to web page requires its html element id, i.e.
 * \code{.html}
 * <input type="text" id="stringProperty">.
 * \endcode
 * Synchronization from web page to Inviwo requires that you add javascript
 * code. Added properties can be linked. Their display name might change but it will not affect
 * their identifier. Example of code to add to HTML-page:
 * \code{.js}
 * <script language="JavaScript">
 * function onTextInput(val) {
 *     window.cefQuery({
 *        request: '<Properties><Property type="org.inviwo.stringProperty"
 *             identifier="PropertySyncExample.stringProperty"><value content="' + val + '"
 *              </Property></Properties>',
 *        onSuccess: function(response) {
 *              document.getElementById("stringProperty").focus();},
 *        onFailure: function(error_code, error_message) {}
 *     });
 * }
 * </script>
 * \endcode
 *
 * ### Inports
 *   * __background__ Background to render web page ontop of.
 *
 * ### Outports
 *   * __webpage__ GUI elements rendered by web browser.
 *
 * ### Properties
 *   * __URL__ Link to webpage, online or file path.
 *   * __Reload__ Fetch page again.
 *   * __Property__ Type of property to add.
 *   * __Html id__ Identifier of html element to synchronize. Not allowed to contain dots, spaces
 * etc.
 *   * __Add property__ Create a property of selected type and identifier. Start to synchronize
 * against loaded webpage.
 */

/**
 * \brief Render webpage into the color and picking layers (OpenGL).
 */
#include <warn/push>
#include <warn/ignore/dll-interface-base>  // Fine if dependent libs use the same CEF lib binaries
#include <warn/ignore/extra-semi>  // Due to IMPLEMENT_REFCOUNTING, remove when upgrading CEF
class IVW_MODULE_WEBBROWSER_API WebBrowserProcessor : public Processor, public CefLoadHandler {
public:
    WebBrowserProcessor(InviwoApplication* app);
    virtual ~WebBrowserProcessor();

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    void deserialize(Deserializer& d) override;

    // Detect when page has loaded
    virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack,
                                      bool canGoForward) override;

    ImageInport background_;
    ImageOutport outport_;

    FileProperty fileName_;
    BoolProperty autoReloadFile_;
    StringProperty url_;     ///< Web page to show
    ButtonProperty reload_;  ///< Force reload url

    DoubleProperty zoom_;

    ButtonProperty runJS_;
    StringProperty js_;

protected:
    std::string getSource();

    enum class SourceType { LocalFile, WebAddress };

    OptionProperty<SourceType> sourceType_;
    BoolProperty isLoading_;

    CEFInteractionHandler cefInteractionHandler_;
    PickingMapper picking_;
    CefImageConverter cefToInviwoImageConverter_;
    // create browser-window
    CefRefPtr<RenderHandlerGL> renderHandler_;
    CefRefPtr<CefBrowser> browser_;

    SingleFileObserver fileObserver_;

    IMPLEMENT_REFCOUNTING(WebBrowserProcessor);
};
#include <warn/pop>
}  // namespace inviwo
