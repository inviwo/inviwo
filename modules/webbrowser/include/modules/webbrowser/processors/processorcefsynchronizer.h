/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2023 Inviwo Foundation
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

#include <inviwo/core/network/processornetworkobserver.h>          // for ProcessorNetworkObserver
#include <inviwo/core/util/dispatcher.h>                           // for Dispatcher
#include <modules/webbrowser/processors/progressbarobservercef.h>  // for ProgressBarObserverCEF

#include <functional>     // for function
#include <map>            // for map
#include <memory>         // for shared_ptr
#include <string>         // for string, operator==, hash
#include <unordered_map>  // for unordered_map

#include <warn/push>
#include <warn/ignore/all>
#include "include/wrapper/cef_message_router.h"
#include <include/base/cef_basictypes.h>  // for int64
#include <include/cef_base.h>             // for CefRefPtr, CefString
#include <include/cef_load_handler.h>     // for CefLoadHandler

class CefBrowser;
class CefFrame;
namespace inviwo {
class Processor;
}  // namespace inviwo

#include <warn/pop>

namespace inviwo {

/** \class ProcessorCefSynchronizer
 * Handles "processor.subscribe.progress" and "parentwebbrowserprocessor" commands sent
 * from the Inviwo javascript API (see webbrowser/data/js/inviwoapi.js).
 *
 * @see PropertyCefSynchronizer
 */
#include <warn/push>
#include <warn/ignore/dll-interface-base>  // Fine if dependent libs use the same CEF lib binaries
#include <warn/ignore/extra-semi>  // Due to IMPLEMENT_REFCOUNTING, remove when upgrading CEF
class IVW_MODULE_WEBBROWSER_API ProcessorCefSynchronizer
    : public CefMessageRouterBrowserSide::Handler,
      public CefLoadHandler,
      public ProcessorNetworkObserver {
public:
    using CallbackFunc = void(const std::string&);
    using CallbackHandle = std::shared_ptr<std::function<CallbackFunc>>;

    /**
     * @param parent web browser processor responsible for the browser. Cannot be null.
     */
    explicit ProcessorCefSynchronizer(const Processor* parent);
    virtual ~ProcessorCefSynchronizer() = default;

    /**
     * Register a \p callback which can be triggered through a cefQuery request where the 'command'
     * is 'callback' and 'name' refers to \p name in the JSON object. The callback will then be
     * called with the payload given by 'data'.
     *
     * \code{.json}
     * {"command": "callback", "callback": "name", "data": "string payload"}
     * \endcode
     *
     * \see OnQuery
     */
    CallbackHandle registerCallback(const std::string& name, std::function<CallbackFunc> callback);

    /**
     * Synchronizes all widgets and sets their frame, called when frame has loaded.
     */
    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                           int httpStatusCode) override;

    /**
     * Called due to cefQuery execution in message_router.html.
     * Expects the request to be a JSON data object, see inviwoapi.js:
     *
     * \code{.json}
     * {"command": "processor.subscribe.progress", "path": ProcessorIdentifier,
     * "onProgressChange":onProgressCallback, "onProgressVisibleChange":onProgressVisibleChange}
     * \endcode
     *
     * or
     * \code{.json}
     * {"command": "parentwebbrowserprocessor"}
     * \endcode
     *
     * or, in case callbacks have been registered, calling the given callback
     * \code{.json}
     * {"command": "callback", "callback": "name", "data" : "string payload"}
     * \endcode
     */
    virtual bool OnQuery(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int64 query_id,
                         const CefString& request, bool persistent,
                         CefRefPtr<Callback> callback) override;

    // ProcessorNetworkObserver override, remove progress observers
    virtual void onProcessorNetworkWillRemoveProcessor(Processor*) override;

private:
    const Processor* parent_;
    std::map<Processor*, ProgressBarObserverCEF> progressObservers_;
    // maps callback name used in JavaScript to registered callbacks
    std::unordered_map<std::string, Dispatcher<CallbackFunc>> callbacks_;
    IMPLEMENT_REFCOUNTING(ProcessorCefSynchronizer);
};
#include <warn/pop>

}  // namespace inviwo
