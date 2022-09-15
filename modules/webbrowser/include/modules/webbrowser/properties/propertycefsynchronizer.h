/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2022 Inviwo Foundation
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

#include <inviwo/core/properties/propertyownerobserver.h>     // for PropertyOwnerObserver
#include <modules/webbrowser/properties/propertywidgetcef.h>  // for PropertyWidgetCEF

#include <cstddef>      // for size_t
#include <memory>       // for unique_ptr
#include <string_view>  // for string_view
#include <vector>       // for vector

#include <warn/push>
#include <warn/ignore/all>
#include "include/wrapper/cef_message_router.h"
#include <include/base/cef_basictypes.h>  // for int64
#include <include/cef_base.h>             // for CefRefPtr, CefString, IMPLE...
#include <include/cef_load_handler.h>     // for CefLoadHandler, CefLoadHand...

class CefBrowser;
class CefFrame;
namespace inviwo {
class Property;
class PropertyWidgetCEFFactory;
}  // namespace inviwo

#include <warn/pop>

namespace inviwo {

/** \class PropertyCefSynchronizer
 * Handles "property.set", "property.get" and "property.subscribe" commands sent
 * from the Inviwo javascript API (see webbrowser/data/js/inviwoapi.js).
 *
 * The path can be to a Processor (myprocessor.myproperty) or
 * system/module Settings (mysetting.myproperty) properties.
 *
 * Flow of information between PropertyWidgetCEF and browser.
 * Changes can start from Inviwo (left) or browser (right).
 * Information is encoded in JSON format, e.g.
 * ```
 * {"command":"subscribe", "path": 'myprocessor.myproperty', "onChange":
 * "onChangeCallbackJS", "propertyObserver": "observerName"}
 * ```
 *     Inviwo           Browser (JavaScript)
 * Property change
 *        |
 * updateFromProperty() --> onChangeCallbackJS(property)
 *                             |
 *                             |
 *     OnQuery  <---------  inviwo.setProperty('myprocessor.myproperty', {value:
 * 2.0});
 *  from_json(json, property);
 *
 *
 */
#include <warn/push>
#include <warn/ignore/dll-interface-base>  // Fine if dependent libs use the same CEF lib binaries
#include <warn/ignore/extra-semi>  // Due to IMPLEMENT_REFCOUNTING, remove when upgrading CEF
class IVW_MODULE_WEBBROWSER_API PropertyCefSynchronizer
    : public CefMessageRouterBrowserSide::Handler,
      public CefLoadHandler,
      public PropertyOwnerObserver {
public:
    /*
     * Only handles events from the provided browser
     */
    explicit PropertyCefSynchronizer(CefRefPtr<CefBrowser> browser,
                                     const PropertyWidgetCEFFactory* htmlWidgetFactory);
    virtual ~PropertyCefSynchronizer() = default;
    /**
     * Removes all property synchronizations.
     */
    virtual void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                             TransitionType transition_type) override;

    /**
     * Synchronizes all widgets and sets their frame, called when frame has loaded.
     */
    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                           int httpStatusCode) override;

    /**
     * Called due to cefQuery execution in message_router.html.
     * Expects the request to be a JSON data object, see inviwoapi.js:
     * {command: "subscribe", "path": propertyPath, "id":htmlId}
     * for synchronizing property to change.
     * {command: "property.set", "path":"PropertyIdentifier", "value":0.5}
     * for setting a value
     * {command: "property.get", "path": propertyPath}
     * for getting a value.
     * Currently only supports a single property in the request.
     * @see PropertyWidgetCEF
     */
    virtual bool OnQuery(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int64 query_id,
                         const CefString& request, bool persistent,
                         CefRefPtr<Callback> callback) override;

    // Remove widget if property is removed
    virtual void onWillRemoveProperty(Property* property, size_t index) override;

private:
    /**
     * Add property to synchronize.
     * Stops synchronizing property when this object
     * is destroyed, property is removed, or when stopSynchronize is called.
     * @param property Property to synchronize
     * @param onChange Callback to execute when the property changes.
     * @param propertyObserverCallback Callback to execute when on PropertyObserver notifications.
     */
    void startSynchronize(Property* property, std::string_view onChangeJS,
                          std::string_view propertyObserverCallbackJS);
    /**
     * Stop property from being synchronized.
     * @param property Property to remove
     */
    void stopSynchronize(Property* property);

    std::vector<std::unique_ptr<PropertyWidgetCEF>> widgets_;
    const PropertyWidgetCEFFactory* htmlWidgetFactory_;  /// Non-owning reference
    int browserIdentifier_;  /// Only handle events with the associated browser
    IMPLEMENT_REFCOUNTING(PropertyCefSynchronizer);
};
#include <warn/pop>

}  // namespace inviwo
