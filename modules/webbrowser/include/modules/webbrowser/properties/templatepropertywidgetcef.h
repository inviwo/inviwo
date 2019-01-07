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

#ifndef IVW_TEMPLATEPROPERTYWIDGETCEF_H
#define IVW_TEMPLATEPROPERTYWIDGETCEF_H

#include <modules/webbrowser/webbrowsermoduledefine.h>
#include <modules/webbrowser/properties/propertywidgetcef.h>
#include <inviwo/core/properties/templateproperty.h>

namespace inviwo {

/**
 * \class TemplatePropertyWidgetCEF
 * \brief CEF property widget for TemplateProperty<T>
 * Parses request and sets property value.
 */
template <typename T>
class TemplatePropertyWidgetCEF : public PropertyWidgetCEF {
public:
    typedef T value_type;
    TemplatePropertyWidgetCEF() = default;
    TemplatePropertyWidgetCEF(TemplateProperty<T>* prop, CefRefPtr<CefFrame> frame = nullptr,
                              std::string htmlId = "")
        : PropertyWidgetCEF(prop, frame, htmlId){};

    virtual bool onQuery(
        CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int64 /*query_id*/,
        const CefString& request, bool /*persistent*/,
        CefRefPtr<CefMessageRouterBrowserSide::Handler::Callback> callback) override {
        // Check if we are blocking queries
        if (onQueryBlocker_ > 0) {
            onQueryBlocker_--;
            callback->Success("");
            return true;
        }
        value_type value;
        const std::string& requestStr = request;
        auto key = std::string(R"("value":")");
        auto start = requestStr.find(key, 0) + key.length();
        auto end = requestStr.find(R"("})", start);
        if (end == std::string::npos) {
            callback->Failure(0, "Failed to parse value " + requestStr);
            return true;
        }
        auto stream = std::stringstream(requestStr.substr(start, end - start));
        if (stream >> value) {
            getProperty()->setInitiatingWidget(this);
            static_cast<TemplateProperty<T>*>(getProperty())->set(value);
            callback->Success("");
            getProperty()->clearInitiatingWidget();
        } else {
            callback->Failure(0, "Failed to parse value " + requestStr);
        }

        return true;
    }
};

}  // namespace inviwo

#endif  // IVW_TEMPLATEPROPERTYWIDGETCEF_H
