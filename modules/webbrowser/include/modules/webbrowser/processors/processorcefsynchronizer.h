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
#include <modules/webbrowser/processors/progressbarobservercef.h>

#include <inviwo/core/network/processornetworkobserver.h>

#include <warn/push>
#include <warn/ignore/all>
#include "include/wrapper/cef_message_router.h"
#include <include/cef_load_handler.h>
#include <warn/pop>

namespace inviwo {

/** \class ProcessorCefSynchronizer
 * Handles "processor.subscribe.progress" commands sent
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
    explicit ProcessorCefSynchronizer() = default;
    virtual ~ProcessorCefSynchronizer() = default;

    /**
     * Synchronizes all widgets and sets their frame, called when frame has loaded.
     */
    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                           int httpStatusCode) override;

    /**
     * Called due to cefQuery execution in message_router.html.
     * Expects the request to be a JSON data object, see inviwoapi.js:
     * {command: "processor.subscribe.progress", "path": ProcessorIdentifier,
     * "onProgressChange":onProgressCallback, "onProgressVisibleChange":onProgressVisibleChange} for
     */
    virtual bool OnQuery(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int64 query_id,
                         const CefString& request, bool persistent,
                         CefRefPtr<Callback> callback) override;

    // ProcessorNetworkObserver override, remove progress observers
    virtual void onProcessorNetworkWillRemoveProcessor(Processor*) override;

private:
    std::map<Processor*, ProgressBarObserverCEF> progressObservers_;
    IMPLEMENT_REFCOUNTING(ProcessorCefSynchronizer);
};
#include <warn/pop>

}  // namespace inviwo
