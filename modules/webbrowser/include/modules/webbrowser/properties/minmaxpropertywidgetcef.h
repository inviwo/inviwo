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

#ifndef IVW_MINMAXPROPERTYWIDGETCEF_H
#define IVW_MINMAXPROPERTYWIDGETCEF_H

#include <modules/webbrowser/webbrowsermoduledefine.h>
#include <modules/webbrowser/properties/templatepropertywidgetcef.h>

#include <inviwo/core/properties/minmaxproperty.h>

namespace inviwo {

/** \class MinMaxPropertyWidgetCEF
 * Widget for synchronizing HTML elements:
 * \code{.html}
 * <input type="range">
 * <input type="number">
 * \endcode
 * where there are min and max attributes.
 * The min and max attributes correspond to MinMaxProperty<T>::value
 * Optionally, there may also be start, end, step and minSeparation attributes.
 *
 * @see MinMaxProperty<T>
 */
template <typename T>
class MinMaxPropertyWidgetCEF : public PropertyWidgetCEF {
public:
    MinMaxPropertyWidgetCEF() = default;

    /**
     * The PropertyWidget will register it self with the property.
     */
    MinMaxPropertyWidgetCEF(MinMaxProperty<T>* property, CefRefPtr<CefFrame> frame = nullptr,
                            std::string htmlId = "");

    /**
     * The PropertyWidget will deregister it self with the property.
     */
    virtual ~MinMaxPropertyWidgetCEF() = default;

    /**
     * Update HTML widget using calls javascript oninput() function on element.
     * Assumes that widget is HTML input attribute.
     */
    virtual void updateFromProperty() override;

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
            glm::tvec2<T> value{j.at("min").get<T>(), j.at("max").get<T>()};
            auto p = static_cast<MinMaxProperty<T>*>(getProperty());

            // Optional parameters
            T rmin = j.count("start") > 0 ? j.at("start").get<T>() : p->getRangeMin();
            T rmax = j.count("end") > 0 ? j.at("end").get<T>() : p->getRangeMax();
            glm::tvec2<T> range{rmin, rmax};
            T increment = j.count("step") > 0 ? j.at("step").get<T>() : p->getIncrement();
            T minSep = j.count("minSeparation") > 0 ? j.at("minSeparation").get<T>()
                                                    : p->getMinSeparation();

            p->setInitiatingWidget(this);
            p->set(value, range, increment, minSep);
            p->clearInitiatingWidget();
            callback->Success("");
        } catch (json::exception& ex) {
            LogError(ex.what());
            callback->Failure(0, ex.what());
        }

        return true;
    }
};

template <typename T>
MinMaxPropertyWidgetCEF<T>::MinMaxPropertyWidgetCEF(MinMaxProperty<T>* property,
                                                    CefRefPtr<CefFrame> frame, std::string htmlId)
    : PropertyWidgetCEF(property, frame, htmlId) {}

template <typename T>
void MinMaxPropertyWidgetCEF<T>::updateFromProperty() {
    // Frame might be null if for example webpage is not found on startup
    if (!PropertyWidgetCEF::frame_) {
        return;
    }
    // LogInfo("updateFromProperty");
    auto property = static_cast<MinMaxProperty<T>*>(this->getProperty());

    std::stringstream script;
    script << "var property = document.getElementById(\"" << this->getHtmlId() << "\");";
    script << "if(property!=null){";
    script << "property.min=" << property->getRangeMin() << ";";
    script << "property.max=" << property->getRangeMax() << ";";
    script << "property.step=" << property->getIncrement() << ";";
    script << "property.start=" << property->getStart() << ";";
    script << "property.end=" << property->getEnd() << ";";
    script << "property.minSeparation=" << property->getMinSeparation() << ";";
    // Send oninput event to update element
    script << "property.oninput();";
    script << "}";
    // Need to figure out how to make sure the frame is drawn after changing values.
    // script << "window.focus();";
    // Block OnQuery, called due to property.oninput()
    this->onQueryBlocker_++;
    this->frame_->ExecuteJavaScript(script.str(), this->frame_->GetURL(), 0);
}

using FloatMinMaxPropertyWidgetCEF = MinMaxPropertyWidgetCEF<float>;

using DoubleMinMaxPropertyWidgetCEF = MinMaxPropertyWidgetCEF<double>;

using IntMinMaxPropertyWidgetCEF = MinMaxPropertyWidgetCEF<int>;
using IntSizeTMinMaxPropertyWidgetCEF = MinMaxPropertyWidgetCEF<size_t>;

using Int64MinMaxPropertyWidgetCEF = MinMaxPropertyWidgetCEF<glm::i64>;
}  // namespace inviwo

#endif  // IVW_ORDINALPROPERTYWIDGETCEF_H
