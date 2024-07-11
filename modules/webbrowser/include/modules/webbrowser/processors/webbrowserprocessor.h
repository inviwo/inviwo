/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2024 Inviwo Foundation
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
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/progressbarowner.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/util/singlefileobserver.h>

#include <modules/webbrowser/processors/webbrowserbase.h>

namespace inviwo {

class InviwoApplication;

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
 */

/**
 * \brief Render webpage into the color and picking layers (OpenGL).
 */
class IVW_MODULE_WEBBROWSER_API WebBrowserProcessor : public Processor, public ProgressBarOwner {
public:
    explicit WebBrowserProcessor(InviwoApplication* app);

    virtual void process() override;

    virtual void deserialize(Deserializer& d) override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    enum class SourceType { LocalFile, WebAddress };

    void updateSource();

    ImageInport background_;
    ImageOutport outport_;

    OptionProperty<SourceType> sourceType_;
    FileProperty fileName_;
    BoolProperty autoReloadFile_;
    StringProperty url_;
    ButtonProperty reload_;
    DoubleProperty zoom_;
    ButtonProperty runJS_;
    StringProperty js_;

    SingleFileObserver fileObserver_;
    WebBrowserBase browser_;
};

}  // namespace inviwo
