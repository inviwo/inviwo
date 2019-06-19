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

#ifndef IVW_ORDINALPROPERTYWIDGETCEF_H
#define IVW_ORDINALPROPERTYWIDGETCEF_H

#include <modules/webbrowser/webbrowsermoduledefine.h>
#include <modules/webbrowser/properties/templatepropertywidgetcef.h>

#include <inviwo/core/properties/ordinalproperty.h>

namespace inviwo {

/** \class OrdinalPropertyWidgetCEF
 * Widget for synchronizing HTML elements:
 * <input type="range">
 * <input type="number">
 */
template <typename T>
class OrdinalPropertyWidgetCEF : public TemplatePropertyWidgetCEF<T> {
public:
    OrdinalPropertyWidgetCEF() = default;

    /**
     * The PropertyWidget will register it self with the property.
     */
    OrdinalPropertyWidgetCEF(OrdinalProperty<T>* property, CefRefPtr<CefFrame> frame = nullptr,
                             std::string htmlId = "");

    /**
     * The PropertyWidget will deregister it self with the property.
     */
    virtual ~OrdinalPropertyWidgetCEF() = default;

    /**
     * Update HTML widget using calls javascript oninput() function on element.
     * Assumes that widget is HTML input attribute.
     */
    virtual void updateFromProperty() override;
};

template <typename T>
OrdinalPropertyWidgetCEF<T>::OrdinalPropertyWidgetCEF(OrdinalProperty<T>* property,
                                                      CefRefPtr<CefFrame> frame, std::string htmlId)
    : TemplatePropertyWidgetCEF<T>(property, frame, htmlId) {}

template <typename T>
void OrdinalPropertyWidgetCEF<T>::updateFromProperty() {
    // Frame might be null if for example webpage is not found on startup
    if (!PropertyWidgetCEF::frame_) {
        return;
    }
    // LogInfo("updateFromProperty");
    auto property = static_cast<OrdinalProperty<T>*>(this->getProperty());

    std::stringstream script;
    json p = *property;
    script << this->getOnChange() << "(" << p.dump() << ");";
    this->onQueryBlocker_++;
    this->frame_->ExecuteJavaScript(script.str(), this->frame_->GetURL(), 0);
}

/**
 * Converts an OrdinalProperty to a JSON object.
 * Produces layout according to the members of OrdinalProperty:
 * { {"value": val}, {"increment": increment},
 *   {"minValue": minVal}, {"maxValue": maxVal}
 * }
 * @see OrdinalProperty
 *
 * Usage example:
 * \code{.cpp}
 * OrdinalProperty<double> p;
 * json j = p;
 * \endcode
 */
template <typename T>
void to_json(json& j, const OrdinalProperty<T>& p) {
    j = json{{"value", p.get()},
             {"minValue", p.getMinValue()},
             {"maxValue", p.getMaxValue()},
             {"increment", p.getIncrement()}};
}

/**
 * Converts a JSON object to an OrdinalProperty.
 * Expects object layout according to the members of OrdinalProperty:
 * { {"value": val}, {"increment": increment},
 *   {"minValue": minVal}, {"maxValue": maxVal}
 * }
 * @see OrdinalProperty
 *
 * Usage example:
 * \code{.cpp}
 * auto p = j.get<OrdinalProperty<double>>();
 * \endcode
 */
template <typename T>
void from_json(const json& j, OrdinalProperty<T>& p) {
    // Extract header and column types
    if (j.empty() || !j.front().is_object()) {
        // Only support object types, i.e. [ {key: value} ]
        return;
    }
    T value = j.count("value") > 0 ? j.at("value").get<T>() : p.get();

    // Optional parameters
    T minVal = j.count("minValue") > 0 ? j.at("minValue").get<T>() : p.getMin();
    T maxVal = j.count("maxValue") > 0 ? j.at("maxValue").get<T>() : p.getMax();
    T increment = j.count("increment") > 0 ? j.at("increment").get<T>() : p.getIncrement();

    p.set(value, minVal, maxVal, increment);
}

using FloatPropertyWidgetCEF = OrdinalPropertyWidgetCEF<float>;

using DoublePropertyWidgetCEF = OrdinalPropertyWidgetCEF<double>;

using IntPropertyWidgetCEF = OrdinalPropertyWidgetCEF<int>;
using IntSizeTPropertyWidgetCEF = OrdinalPropertyWidgetCEF<size_t>;

using Int64PropertyWidgetCEF = OrdinalPropertyWidgetCEF<glm::i64>;

// Scalar properties
extern template class IVW_MODULE_WEBBROWSER_TMPL_EXP OrdinalPropertyWidgetCEF<float>;
extern template class IVW_MODULE_WEBBROWSER_TMPL_EXP OrdinalPropertyWidgetCEF<int>;
extern template class IVW_MODULE_WEBBROWSER_TMPL_EXP OrdinalPropertyWidgetCEF<size_t>;
extern template class IVW_MODULE_WEBBROWSER_TMPL_EXP OrdinalPropertyWidgetCEF<glm::i64>;
extern template class IVW_MODULE_WEBBROWSER_TMPL_EXP OrdinalPropertyWidgetCEF<double>;
}  // namespace inviwo

#endif  // IVW_ORDINALPROPERTYWIDGETCEF_H
