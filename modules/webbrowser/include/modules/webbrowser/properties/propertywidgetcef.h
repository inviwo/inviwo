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

#ifndef IVW_PROPERTYWIDGETCEF_H
#define IVW_PROPERTYWIDGETCEF_H

#include <modules/webbrowser/webbrowsermoduledefine.h>
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
 * \brief Base class for HTML widgets.
 * Parses JSON-formated message and sets property values on the Inviwo-side and
 * executes javascript on the HTML-side.
 * Example code on HTML-side:
 * \code{.html}
 * <input type="range" class="slider" id="PropertyIdentifier">
 * <script>
 * var slider = document.getElementById("PropertyIdentifier");
 * slider.oninput = function() {
 *     window.cefQuery({
 *        request: '{"id":"PropertyIdentifier", "value":"' +  slider.value + '"}',
 *        onSuccess: function(response) {},
 *        onFailure: function(error_code, error_message) {}
 *     });
 * }
 * </script>
 * \endcode
 * Subclasses should override updateFromProperty() and use javascript
 * to send values to HTML-elements.
 * HTML-element reference:
 * https://www.w3schools.com/html/html_form_elements.asp
 *
 * @note Property serialization cannot be used to implement synchronization since
 * it for example changes the property identifier if not set.
 * @see TemplatePropertyWidgetCEF
 */
class IVW_MODULE_WEBBROWSER_API PropertyWidgetCEF : public PropertyWidget,
                                                    public PropertyObserver,
                                                    public Serializable {
public:
    PropertyWidgetCEF() = default;
    PropertyWidgetCEF(Property* prop, CefRefPtr<CefFrame> frame = nullptr, std::string htmlId = "");

    friend class CefDOMSearchId;
    friend class PropertyCefSynchronizer;

    virtual ~PropertyWidgetCEF() = default;

    /*
     * Checks if frame contains the html id of the widget and sets it if it is.
     * CefFrame is required for communication between Inviwo and the web browser.
     */
    void setFrameIfPartOfFrame(CefRefPtr<CefFrame> frame);
    /*
     * Set id of corresponding element in HTML-webpage.
     * @param id HTML element id in webpage.
     */
    void setHtmlId(std::string id) { htmlId_ = id; }
    const std::string& getHtmlId() const { return htmlId_; }

    /*
     * Sets property value given by JSON-formated request if onQueryBlocker_ > 0,
     * otherwise decrements onQueryBlocker_ and returns true.
     *
     * Called from PropertyCefSynchronizer when cefQuery execution
     * includes {"id":"htmlId"} property path request.
     * Calls callback->Success("") if property is deserialized
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
                         CefRefPtr<CefMessageRouterBrowserSide::Handler::Callback> callback) = 0;

    virtual void serialize(Serializer&) const override{};
    virtual void deserialize(Deserializer& d) override;

protected:
    // PropertyObservable overrides
    virtual void onSetReadOnly(Property* property, bool readonly) override;
    /*
     * Set frame containing html item.
     */
    void setFrame(CefRefPtr<CefFrame> frame);

    std::string htmlId_;         /// Id in used in html, usually Processor.PropertyId
    CefRefPtr<CefFrame> frame_;  /// Browser frame containing corresponding properties
    int onQueryBlocker_ = 0;     /// Block jacascript callback queries
};

}  // namespace inviwo

#endif  // IVW_PROPERTYWIDGETCEF_H
