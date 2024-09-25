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

#include <modules/webbrowser/webbrowsermoduledefine.h>  // for IVW_MODULE_WEBBROWSE...
#include <inviwo/core/common/inviwomodule.h>            // for InviwoModule
#include <inviwo/core/util/timer.h>                     // for Timer
#include <modules/webbrowser/webbrowserclient.h>        // for WebBrowserClient

#include <memory>   // for unique_ptr, make_unique
#include <string>   // for string
#include <utility>  // for move
#include <vector>   // for vector

#include <warn/push>
#include <warn/ignore/all>
#include <include/cef_base.h>  // for CefRefPtr
#if __APPLE__                  // Mac
#include "include/wrapper/cef_library_loader.h"
#endif
#include <warn/pop>

namespace inviwo {
class InviwoApplication;

/*
 * Enable suppport for showing web pages using CEF
 */
class IVW_MODULE_WEBBROWSER_API WebBrowserModule : public InviwoModule {
public:
    WebBrowserModule(InviwoApplication* app);
    virtual ~WebBrowserModule();

    CefRefPtr<WebBrowserClient> getBrowserClient() { return browserClient_; }

protected:
    CefRefPtr<WebBrowserClient> browserClient_;

    Timer doChromiumWork_;  /// Calls CefDoMessageLoopWork()
#ifdef __APPLE__            // Load library dynamically for Mac
    CefScopedLibraryLoader cefLib_;
#endif
};

}  // namespace inviwo
