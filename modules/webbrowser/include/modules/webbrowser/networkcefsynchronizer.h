/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <inviwo/core/processors/processor.h>  // for Processor
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/util/settings/settings.h>
#include <inviwo/core/processors/progressbarowner.h>

#include <modules/webbrowser/processors/progressbarobservercef.h>

#include <modules/json/jsonpropertyconverter.h>
#include <modules/json/jsoninportconverter.h>
#include <modules/json/jsonoutportconverter.h>

#include <warn/push>
#include <warn/ignore/all>
#include <include/wrapper/cef_message_router.h>
#include <include/cef_load_handler.h>
#include <include/cef_base.h>
#include <warn/pop>

#include <nlohmann/json.hpp>

#include <string>
#include <string_view>

namespace inviwo {

using json = ::nlohmann::json;

class InviwoApplication;
class PropertyWidgetCEF;

#include <warn/push>
#include <warn/ignore/dll-interface-base>
class IVW_MODULE_WEBBROWSER_API NetWorkCefSynchronizer
    : public CefMessageRouterBrowserSide::Handler,
      public CefLoadHandler,
      public PropertyOwnerObserver,
      public ProcessorNetworkObserver {
public:
    using CallbackFunc = std::string(const std::string&);
    using CallbackHandle = std::shared_ptr<std::function<CallbackFunc>>;

    /*
     * Only handles events from the provided browser
     */
    explicit NetWorkCefSynchronizer(InviwoApplication* app);

    virtual ~NetWorkCefSynchronizer();

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
     * Expects the request to be a JSON data object, see inviwoapiv2.js:
     *
     * * for synchronizing property to change:
     *      {command: "subscribe", "path": propertyPath, "id":htmlId}
     * * for setting a value;
     *      {command: "property.set", "path":"PropertyIdentifier", "value":0.5}
     * * for getting a value:
     *      {command: "property.get", "path": propertyPath}
     *
     * * Network commands:
     *   * {command: "network.processors"}
     * * Processor commands:
     *   * {command: "processor.properties", identifier: "ProcessorIdentifier"}
     *   * {command: "processor.inports",    identifier: "ProcessorIdentifier"}
     *   * {command: "processor.outports",   identifier: "ProcessorIdentifier"}
     * * Property commands:
     *  * {command: "property.get", path: "ProcessorIdentifier/PropertyIdentifier"}
     *  * {command: "property.set", path: "ProcessorIdentifier/PropertyIdentifier", "data": 0.5}
     * * Inport commands:
     *  * {command: "inport.getData", processor: "ProcessorIdentifier", identifier:
     * "InportIdentifier"}
     * * BrushingAndLinkingInport commands:
     *   * {command: "inport.getFilteredIndices",
     *      processor: "ProcessorIdentifier",
     *      identifier: "InportIdentifier"
     *      target: "BrushingTarget"}
     *   * {command: "inport.getSelectedIndices",
     *      processor: "ProcessorIdentifier",
     *      identifier: "InportIdentifier"
     *      target: "BrushingTarget"}
     *   * {command: "inport.getHighlightedIndices",
     *      processor: "ProcessorIdentifier",
     *      identifier: "InportIdentifier"
     *      target: "BrushingTarget"}
     *   * {command: "inport.filter",
     *      processor: "ProcessorIdentifier",
     *      identifier: "InportIdentifier"
     *      target: "BrushingTarget"
     *      indices: "list of indices"}
     *    * {command: "inport.select",
     *      processor: "ProcessorIdentifier",
     *      identifier: "InportIdentifier"
     *      target: "BrushingTarget"
     *      indices: "list of indices"}
     *    * {command: "inport.highlight",
     *      processor: "ProcessorIdentifier",
     *      identifier: "InportIdentifier"
     *      target: "BrushingTarget"
     *      indices: "list of indices"}
     * * Outport commands:
     *   * {command: "outport.setData",
     *      processor: "ProcessorIdentifier",
     *      identifier: "OutportIdentifier",
     *      data: "data"}
     *
     */
    virtual bool OnQuery(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int64_t query_id,
                         const CefString& request, bool persistent,
                         CefRefPtr<Callback> callback) override;

    // Remove widget if property is removed
    virtual void onWillRemoveProperty(Property* property, size_t index) override;

    virtual void onProcessorNetworkWillRemoveProcessor(Processor* p) override;

    void clear(CefRefPtr<CefBrowser> browser);

    CallbackHandle registerCallback(CefRefPtr<CefBrowser> browser, const std::string& name,
                                    std::function<CallbackFunc> callback);

private:
    using Callback = CefMessageRouterBrowserSide::Handler::Callback;

    /**
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
     * {"command":"subscribe", "path": "myprocessor.myproperty", "onChange":
     * "onChangeCallbackJS", "propertyObserver": "observerName"}
     * ```
     *     Inviwo           Browser (JavaScript)
     * Property change
     *        |
     * updateFromProperty() --> onChangeCallbackJS(property)
     *                             |
     *                             |
     *     OnQuery  <---------  inviwo.setProperty("myprocessor.myproperty", {value: 2.0});
     *  from_json(json, property);
     */
    bool propertySubscribe(const json& j, CefRefPtr<CefBrowser>& browser,
                           CefRefPtr<CefFrame>& frame, CefRefPtr<Callback>& callback);

    bool propertyAction(const json& j, CefRefPtr<CefBrowser>& browser, CefRefPtr<CefFrame>& frame,
                        int64_t query_id, const CefString& request, bool persistent,
                        CefRefPtr<Callback>& callback);
    bool handleCallback(const json& j, CefRefPtr<CefBrowser>& browser,
                        CefRefPtr<Callback>& callback);

    bool handleNetwork(const json& j, CefRefPtr<CefBrowser>& browser, CefRefPtr<Callback>& reponse);
    bool handleProcessor(const json& j, CefRefPtr<CefBrowser>& browser,
                         CefRefPtr<Callback>& reponse);
    bool handleInport(const json& j, CefRefPtr<CefBrowser>& browser, CefRefPtr<Callback>& reponse);
    bool handleOutport(const json& j, CefRefPtr<CefBrowser>& browser, CefRefPtr<Callback>& reponse);
    bool handleProperty(const json& j, CefRefPtr<CefBrowser>& browser,
                        CefRefPtr<Callback>& reponse);

    bool processorUnsubscribeProgress(const json& j, CefRefPtr<Callback>& callback);

    bool processorSubscribeProgress(const json& j, CefRefPtr<CefFrame>& frame,
                                    CefRefPtr<Callback>& callback);

    // Searches first for the property in Processors and then in Settings
    Property* findProperty(std::string_view path);

    /**
     * Add property to synchronize.
     * Stops synchronizing property when this object
     * is destroyed, property is removed, or when stopSynchronize is called.
     * @param property Property to synchronize
     * @param frame Frame to synchronize with
     * @param onChangeJS Callback to execute when the property changes.
     * @param propertyObserverCallbackJS Callback to execute when on PropertyObserver notifications.
     */
    std::unique_ptr<PropertyWidgetCEF> createWidget(Property* property, CefRefPtr<CefFrame>& frame,
                                                    std::string_view onChangeJS,
                                                    std::string_view propertyObserverCallbackJS);

    InviwoApplication* app_;
    const JSONPropertyConverter& jsonPropertyConverter_;  /// Non-owning reference
    const JSONInportConverter& jsonInportConverter_;      /// Non-owning reference
    const JSONOutportConverter& jsonOutportConverter_;    /// Non-owning reference

    std::map<int, std::vector<std::unique_ptr<PropertyWidgetCEF>>> widgets_;

    std::map<Processor*, ProgressBarObserverCEF> progressObservers_;

    std::map<int, std::map<std::string, std::weak_ptr<std::function<CallbackFunc>>>> callbacks_;

    IMPLEMENT_REFCOUNTING(NetWorkCefSynchronizer);
};
#include <warn/pop>

}  // namespace inviwo
