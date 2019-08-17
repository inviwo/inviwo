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

#ifndef IVW_PROPERTYWIDGETCEF_H
#define IVW_PROPERTYWIDGETCEF_H

#include <modules/webbrowser/webbrowsermoduledefine.h>
#include <modules/json/io/json/propertyjsonconverter.h>
#include <inviwo/core/io/serialization/serializable.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/propertywidget.h>
#include <inviwo/core/properties/propertyobserver.h>

#include <warn/push>
#include <warn/ignore/all>
#include <include/cef_frame.h>
#include "include/wrapper/cef_message_router.h"
#include <warn/pop>

namespace inviwo {

/**
 * \class PropertyWidgetCEF
 * \brief Handler for setting, getting, onChange and PropertyObservable of a property from HTML.
 * Handles "property.set", "property.get" commands sent from the Inviwo javascript API (see
 * webbrowser/data/js/inviwoapi.js) and sets property values on the Inviwo-side.
 *
 * PropertyWidgetCEF must have a PropertyJSONConverter for its corresponding property.
 * Thus, to add support for a new property it is only necessary to:
 * 1. Implement to_json and from_json
 * 2. Register converter using
 * WebbrowserModule::registerPropertyJSONConverterAndWidget<PropertyWidgetCEF, MyNewProperty>()
 *
 * Example code on HTML-side:
 * \code{.html}
 * <script language="JavaScript">
 * // Initialize Inviwo API so that we can use it to synchronize properties
 * var inviwo = new InviwoAPI();
 * </script>
 * <input type="range" class="slider" id="PropertyIdentifier"
 * oninput="inviwo.setProperty('MyProcessor.MyProperty', {value: Number(this.value)})"> \endcode
 *
 * @note Property serialization cannot be used to implement synchronization since
 * it for example changes the property identifier if not set.
 */
class IVW_MODULE_WEBBROWSER_API PropertyWidgetCEF : public PropertyWidget, public PropertyObserver {
public:
    PropertyWidgetCEF() = default;
    PropertyWidgetCEF(Property* prop, std::unique_ptr<PropertyJSONConverter> converter,
                      CefRefPtr<CefFrame> frame = nullptr, std::string onChange = "");

    friend class CefDOMSearchId;
    friend class PropertyCefSynchronizer;

    virtual ~PropertyWidgetCEF() = default;

    /*
     * Checks if frame contains the html id of the widget and sets it if it is.
     * CefFrame is required for communication between Inviwo and the web browser.
     */
    void setFrameIfPartOfFrame(CefRefPtr<CefFrame> frame);

    void setOnChange(std::string onChange) { onChange_ = onChange; }
    const std::string& getOnChange() const { return onChange_; }

    void setPropertyObserverCallback(std::string propertyObserverCallback) {
        propertyObserverCallback_ = propertyObserverCallback;
    }
    const std::string& getPropertyObserverCallback() const { return propertyObserverCallback_; }

    /*
     * Handles "property.set" and "property.get" commands given by JSON-formated request.
     *
     * Return true to handle the query
     * or false to propagate the query to other registered handlers, if any. If
     * no handlers return true from this method then the query will be
     * automatically canceled with an error code of -1 delivered to the
     * JavaScript onFailure callback. If this method returns true then a
     * Callback method must be executed either in this method or asynchronously
     * to complete the query.
     */
    virtual bool onQuery(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int64 query_id,
                         const CefString& request, bool persistent,
                         CefRefPtr<CefMessageRouterBrowserSide::Handler::Callback> callback);

    /**
     * Calls the currently set onChange function in javascript with the JSON
     * encoded property as parameter.
     * The onChange javascript function must be in global scope.
     * @see setOnChange
     */
    virtual void updateFromProperty() override;

protected:
    // PropertyObservable overrides
    /**
     * Calls the currently set propertyObserverCallback function in javascript with the JSON
     * encoded changed value as parameter.
     * The propertyObserverCallback javascript function must be in global scope.
     * @see setPropertyObserverCallback
     */
    virtual void onSetIdentifier(Property* property, const std::string& identifier) override;
    /**
     * Calls the currently set propertyObserverCallback function in javascript with the JSON
     * encoded changed value as parameter.
     * The propertyObserverCallback javascript function must be in global scope.
     * @see setPropertyObserverCallback
     */
    virtual void onSetDisplayName(Property* property, const std::string& displayName) override;
    /**
     * Calls the currently set propertyObserverCallback function in javascript with the JSON
     * encoded changed value as parameter.
     * The propertyObserverCallback javascript function must be in global scope.
     * @see setPropertyObserverCallback
     */
    virtual void onSetSemantics(Property* property, const PropertySemantics& semantics) override;
    /**
     * Calls the currently set propertyObserverCallback function in javascript with the JSON
     * encoded changed value as parameter.
     * The propertyObserverCallback javascript function must be in global scope.
     * @see setPropertyObserverCallback
     */
    virtual void onSetReadOnly(Property* property, bool readonly) override;
    /**
     * Calls the currently set propertyObserverCallback function in javascript with the JSON
     * encoded changed value as parameter.
     * The propertyObserverCallback javascript function must be in global scope.
     * @see setPropertyObserverCallback
     */
    virtual void onSetVisible(Property* property, bool visible) override;
    /**
     * Calls the currently set propertyObserverCallback function in javascript with the JSON
     * encoded changed value as parameter.
     * The propertyObserverCallback javascript function must be in global scope.
     * @see setPropertyObserverCallback
     */
    virtual void onSetUsageMode(Property* property, UsageMode usageMode) override;
    /*
     * Set frame containing html item.
     */
    void setFrame(CefRefPtr<CefFrame> frame);

    std::unique_ptr<PropertyJSONConverter> converter_;

    std::string onChange_;  /// Callback to execute in javascript when property changes
    std::string propertyObserverCallback_;  /// Execute on any PropertyObserver notifications
    CefRefPtr<CefFrame> frame_;             /// Browser frame containing corresponding callbacks
};

}  // namespace inviwo

#endif  // IVW_PROPERTYWIDGETCEF_H
