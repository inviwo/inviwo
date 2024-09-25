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

#include <modules/webbrowser/webbrowserapp.h>

#include <inviwo/core/util/logcentral.h>

#include <include/base/cef_scoped_refptr.h>  // for scoped_refptr
#include <include/cef_base.h>                // for CefRefPtr, CefString
#include <include/cef_command_line.h>        // for CefCommandLine

#include <include/cef_browser.h>
#include <include/cef_callback.h>
#include <include/cef_frame.h>
#include <include/cef_request.h>
#include <include/cef_resource_handler.h>
#include <include/cef_response.h>
#include <include/cef_scheme.h>
#include <include/wrapper/cef_helpers.h>

namespace inviwo {

WebBrowserApp::WebBrowserApp() = default;

void WebBrowserApp::OnBeforeCommandLineProcessing(const CefString&,
                                                  CefRefPtr<CefCommandLine> command_line) {
    // Enable loading files from other locations than where the .html file is
    // see https://bitbucket.org/chromiumembedded/cef/commits/f158c34
    command_line->AppendSwitch("allow-file-access-from-files");
    // Avoid pop-up when application starts:
    // "Inviwo wants to use your confidential information stored in
    // Chromium Safe Storage in your keychain"
    // https://bitbucket.org/chromiumembedded/cef/issues/2692/mac-networkservice-allow-custom-service
    command_line->AppendSwitch("use-mock-keychain");
}

}  // namespace inviwo
