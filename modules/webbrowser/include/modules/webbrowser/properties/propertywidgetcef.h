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

#include <modules/webbrowser/webbrowsermoduledefine.h>  // for IVW_MODULE_WEBBROWSER_API

#include <inviwo/core/properties/propertyobserver.h>  // for PropertyObserver
#include <inviwo/core/properties/propertywidget.h>    // for PropertyWidget
#include <modules/json/jsonpropertyconverter.h>
#include <warn/push>
#include <warn/ignore/all>
#include "include/wrapper/cef_message_router.h"
#include <include/cef_base.h>   // for CefRefPtr, CefString
#include <include/cef_frame.h>  // for CefFrame
#include <warn/pop>

#include <memory>       // for unique_ptr
#include <string>       // for string
#include <string_view>  // for string_view

class CefBrowser;

namespace inviwo {

class Property;
class PropertySemantics;

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
    PropertyWidgetCEF(Property* prop, const JSONPropertyConverter& converter,
                      CefRefPtr<CefFrame> frame = nullptr, std::string_view onChange = "");

    friend class CefDOMSearchId;
    friend class PropertyCefSynchronizer;

    virtual ~PropertyWidgetCEF() = default;

    /*
     * Set the frame containing the onChange function.
     * Calls onChange if frame is not null.
     * CefFrame is required for communication between Inviwo and the web browser.
     */
    void setFrame(CefRefPtr<CefFrame> frame);

    void setOnChange(std::string_view onChange) { onChange_ = onChange; }
    const std::string& getOnChange() const { return onChange_; }

    void setPropertyObserverCallback(std::string_view propertyObserverCallback) {
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
    virtual bool onQuery(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int64_t query_id,
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

    const JSONPropertyConverter& converter_;

    std::string onChange_;  /// Callback to execute in javascript when property changes
    std::string propertyObserverCallback_;  /// Execute on any PropertyObserver notifications
    CefRefPtr<CefFrame> frame_;             /// Browser frame containing corresponding callbacks
};

}  // namespace inviwo
