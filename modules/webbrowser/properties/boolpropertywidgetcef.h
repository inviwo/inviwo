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

#ifndef IVW_BOOLPROPERTYWIDGETCEF_H
#define IVW_BOOLPROPERTYWIDGETCEF_H

#include <modules/webbrowser/webbrowsermoduledefine.h>
#include <modules/webbrowser/properties/propertywidgetcef.h>

#include <inviwo/core/properties/boolproperty.h>

namespace inviwo {

/**
 * \class BoolPropertyWidgetCEF
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 * DESCRIBE_THE_CLASS_FROM_A_DEVELOPER_PERSPECTIVE
 */
class IVW_MODULE_WEBBROWSER_API BoolPropertyWidgetCEF : public PropertyWidgetCEF {
public:
    BoolPropertyWidgetCEF(BoolProperty* property = nullptr, CefRefPtr<CefFrame> frame = nullptr, std::string htmlId = "");
    virtual ~BoolPropertyWidgetCEF() = default;
    /**
     * Update HTML widget using calls javascript oninput() function on element.
     * Assumes that widget is HTML input attribute.
     */
    virtual void updateFromProperty() {
        //LogInfo("updateFromProperty");
        auto property = static_cast<BoolProperty*>(property_);
        
        std::stringstream script;
        script << "var property = document.getElementById(\"" << htmlId_ << "\");";
        // Use click instead of setting value to make sure that appropriate events are fired.
        script << "if (property.checked !=" << (property->get() ? "true" : "false") << "){property.click();}";
        // Need to figure out how to make sure the frame is drawn after changing values.
        //script << "window.focus();";
        // Block OnQuery, called due to property.oninput()
        onQueryBlocker_++;
        frame_->ExecuteJavaScript(script.str(),
                                  frame_->GetURL(), 0);
    };
};

}  // namespace inviwo

#endif  // IVW_BOOLPROPERTYWIDGETCEF_H
