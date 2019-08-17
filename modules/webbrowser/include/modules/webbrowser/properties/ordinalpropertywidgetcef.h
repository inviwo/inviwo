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
 * \code{.html}
 * <input type="range">
 * <input type="number">
 * \endcode
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
    script << "var property = document.getElementById(\"" << this->getHtmlId() << "\");";
    script << "if(property!=null){";
    script << "property.min=" << property->getMinValue() << ";";
    script << "property.max=" << property->getMaxValue() << ";";
    script << "property.step=" << property->getIncrement() << ";";
    script << "property.value=" << property->get() << ";";
    // Send oninput event to update element
    script << "property.oninput();";
    script << "}";
    // Need to figure out how to make sure the frame is drawn after changing values.
    // script << "window.focus();";
    // Block OnQuery, called due to property.oninput()
    this->onQueryBlocker_++;
    this->frame_->ExecuteJavaScript(script.str(), this->frame_->GetURL(), 0);
}

using FloatPropertyWidgetCEF = OrdinalPropertyWidgetCEF<float>;

using DoublePropertyWidgetCEF = OrdinalPropertyWidgetCEF<double>;

using IntPropertyWidgetCEF = OrdinalPropertyWidgetCEF<int>;
using IntSizeTPropertyWidgetCEF = OrdinalPropertyWidgetCEF<size_t>;

using Int64PropertyWidgetCEF = OrdinalPropertyWidgetCEF<glm::i64>;
}  // namespace inviwo

#endif  // IVW_ORDINALPROPERTYWIDGETCEF_H
