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

#ifndef IVW_WEBBROWSERAPP_H
#define IVW_WEBBROWSERAPP_H

#include <modules/webbrowser/webbrowsermoduledefine.h>

#include <warn/push>
#include <warn/ignore/all>
#include <include/cef_app.h>
#include <include/wrapper/cef_helpers.h>
#include <include/wrapper/cef_message_router.h>
#include <warn/pop>

namespace inviwo {

/**
 * App to be used in the browser thread.
 */
#include <warn/push>
#include <warn/ignore/dll-interface-base>  // Fine if dependent libs use the same CEF lib binaries
#include <warn/ignore/extra-semi>  // Due to IMPLEMENT_REFCOUNTING, remove when upgrading CEF
class IVW_MODULE_WEBBROWSER_API WebBrowserApp : public CefApp, public CefBrowserProcessHandler {
public:
    WebBrowserApp();
    // Provides an opportunity to view and/or modify command-line arguments before
    // processing by CEF and Chromium. The |process_type| value will be empty for
    // the browser process. Do not keep a reference to the CefCommandLine object
    // passed to this method. The CefSettings.command_line_args_disabled value
    // can be used to start with an empty command-line object. Any values
    // specified in CefSettings that equate to command-line arguments will be set
    // before this method is called. Be cautious when using this method to modify
    // command-line arguments for non-browser processes as this may result in
    // undefined behavior including crashes.
    //
    // Inviwo: Added allow-file-access-from-files to avoid
    //"Blocked a frame with origin "null" from
    // accessing a frame with origin "null". Protocols, domains, and ports must match."
    // This can happen when trying to access a newly opened dialog
    ///
    /*--cef(optional_param=process_type)--*/
    virtual void OnBeforeCommandLineProcessing(const CefString& process_type,
                                               CefRefPtr<CefCommandLine> command_line) OVERRIDE;
    // CefBrowserProcessHandler methods:
    CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() OVERRIDE { return this; }

private:
    IMPLEMENT_REFCOUNTING(WebBrowserApp);
};
#include <warn/pop>

};      // namespace inviwo
#endif  // IVW_WEBBROWSERAPP_H
