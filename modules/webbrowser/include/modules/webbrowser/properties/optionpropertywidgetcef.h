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

#pragma once

#include <modules/webbrowser/webbrowsermoduledefine.h>
#include <modules/webbrowser/properties/propertywidgetcef.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/util/raiiutils.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace inviwo {
// TODO: Remove when moved to c++17
#if __cplusplus < 201703L
namespace detail {
template <class T>
json toJSON(TemplateOptionProperty<T>* p,
            typename std::enable_if<std::is_same<T, bool>::value>::type* = 0) {
    json res = {{"value", p->get() ? "true" : "false"}};
    return res;
}
template <class T>
json toJSON(TemplateOptionProperty<T>* p,
            typename std::enable_if<!std::is_same<T, bool>::value>::type* = 0) {
    json res = {{"value", p->get()}};
    return res;
}

}  // namespace detail
#endif
/**
 * \class TemplateOptionPropertyWidgetCEF
 * \brief CEF property widget for TemplateOptionProperty<T>
 * Parses request and sets property value.
 */
template <typename T>
class TemplateOptionPropertyWidgetCEF : public PropertyWidgetCEF {
public:
    typedef T value_type;
    TemplateOptionPropertyWidgetCEF() = default;
    TemplateOptionPropertyWidgetCEF(TemplateOptionProperty<T>* prop,
                                    CefRefPtr<CefFrame> frame = nullptr, std::string htmlId = "")
        : PropertyWidgetCEF(prop, frame, htmlId){};
    virtual ~TemplateOptionPropertyWidgetCEF() = default;

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
        const std::string& requestString = request;
        auto j = json::parse(requestString);
        try {
            auto command = j.at("command").get<std::string>();
            auto p = static_cast<TemplateOptionProperty<T>*>(getProperty());
            if (command.rfind("property.set", 0) == 0) {
                try {
                    p->setInitiatingWidget(this);
                    util::OnScopeExit clearInitiatingWidget([p]() { p->clearInitiatingWidget(); });
                    json ret; // Return value
                    if (command == "property.set") {
                        T value = j.at("value").get<T>();
                        p->set(value);
                    } else if (command == "property.setSelectedIndex") {
                        auto value = j.at("value").get<size_t>();
                        ret = {"value",  p->setSelectedIndex(value) ? "true" : "false"};
                    } else if (command == "property.setSelectedIdentifier") {
                        auto value = j.at("value").get<std::string>();
                        ret = {"value",  p->setSelectedIdentifier(value) ? "true" : "false"};
                    } else if (command == "property.setSelectedDisplayName") {
                        auto value = j.at("value").get<std::string>();
                        ret = {"value",  p->setSelectedDisplayName(value) ? "true" : "false"};
                    }
                    callback->Success(ret.dump());
                } catch (json::exception& ex) {
                    LogError(ex.what());
                    callback->Failure(0, ex.what());
                }
            } else if (command.rfind("property.get", 0) == 0) {
                if (command == "property.get") {
// TODO: Remove when moved to c++17
#if __cplusplus >= 201703L
                    if constexpr (std::is_same_v<T, bool>) {
                        json res = {"value", p->get() ? "true" : "false"};
                        callback->Success(res.dump());
                    } else {
                        json res = {"value", p->get()};
                        callback->Success(res.dump());
                    }
#else
                    json res = detail::toJSON(p);
                    callback->Success(res.dump());
#endif
                }
            }
        } catch (json::exception& ex) {
            LogError(ex.what());
            callback->Failure(0, ex.what());
        }

        return true;
    }

    /**
     * Update HTML widget using calls javascript oninput() function on element.
     * Assumes that widget is HTML input attribute.
     */
    virtual void updateFromProperty() override;
};


template <typename T>
void TemplateOptionPropertyWidgetCEF<T>::updateFromProperty() {
    // Frame might be null if for example webpage is not found on startup
    if (!PropertyWidgetCEF::frame_) {
        return;
    }
    // LogInfo("updateFromProperty");
    auto property = static_cast<TemplateOptionProperty<T>*>(this->getProperty());

    std::stringstream script;
    json p = *property;
    script << this->getOnChange() << "(" << p.dump() << ");";
    //script << "var property = document.getElementById(\"" << this->getHtmlId() << "\");";
    //script << "if(property!=null){";
    //script << "property.value=" << property->get() << ";";
    //// Send oninput event to update element
    //script << "property.oninput();";
    //script << "}";
    // Need to figure out how to make sure the frame is drawn after changing values.
    // script << "window.focus();";
    // Block OnQuery, called due to property.oninput()
    this->onQueryBlocker_++;
    this->frame_->ExecuteJavaScript(script.str(), this->frame_->GetURL(), 0);
}


using OptionPropertyWidgetCEFUInt = TemplateOptionPropertyWidgetCEF<unsigned int>;
using OptionPropertyWidgetCEFInt = TemplateOptionPropertyWidgetCEF<int>;
using OptionPropertyWidgetCEFSize_t = TemplateOptionPropertyWidgetCEF<size_t>;
using OptionPropertyWidgetCEFFloat = TemplateOptionPropertyWidgetCEF<float>;
using OptionPropertyWidgetCEFDouble = TemplateOptionPropertyWidgetCEF<double>;
using OptionPropertyWidgetCEFString = TemplateOptionPropertyWidgetCEF<std::string>;

extern template class IVW_MODULE_WEBBROWSER_TMPL_EXP TemplateOptionPropertyWidgetCEF<unsigned int>;
extern template class IVW_MODULE_WEBBROWSER_TMPL_EXP TemplateOptionPropertyWidgetCEF<int>;
extern template class IVW_MODULE_WEBBROWSER_TMPL_EXP TemplateOptionPropertyWidgetCEF<size_t>;
extern template class IVW_MODULE_WEBBROWSER_TMPL_EXP TemplateOptionPropertyWidgetCEF<float>;
extern template class IVW_MODULE_WEBBROWSER_TMPL_EXP TemplateOptionPropertyWidgetCEF<double>;
extern template class IVW_MODULE_WEBBROWSER_TMPL_EXP TemplateOptionPropertyWidgetCEF<std::string>;

}  // namespace inviwo
