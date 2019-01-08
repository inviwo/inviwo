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

#include <modules/webbrowser/properties/boolpropertywidgetcef.h>

namespace inviwo {

BoolPropertyWidgetCEF::BoolPropertyWidgetCEF(BoolProperty* property, CefRefPtr<CefFrame> frame,
                                             std::string htmlId)
    : TemplatePropertyWidgetCEF<bool>(property, frame, htmlId) {}

void BoolPropertyWidgetCEF::updateFromProperty() {
    // Frame might be null if for example webpage is not found on startup
    if (!frame_) {
        return;
    }
    // LogInfo("updateFromProperty");
    auto property = static_cast<BoolProperty*>(property_);

    std::stringstream script;
    script << "var property = document.getElementById(\"" << htmlId_ << "\");";
    // Use click instead of setting value to make sure that appropriate events are fired.
    script << "if (property!=null && property.checked !=" << (property->get() ? "true" : "false")
           << "){property.click();}";
    // Need to figure out how to make sure the frame is drawn after changing values.
    // script << "window.focus();";
    // Block OnQuery, called due to property.oninput()
    onQueryBlocker_++;
    frame_->ExecuteJavaScript(script.str(), frame_->GetURL(), 0);
}

}  // namespace inviwo
