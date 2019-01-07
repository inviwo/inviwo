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

#ifndef IVW_WEBBROWSERMODULE_H
#define IVW_WEBBROWSERMODULE_H

#include <modules/webbrowser/webbrowsermoduledefine.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/util/timer.h>

#include <warn/push>
#include <warn/ignore/all>

#include "include/internal/cef_types.h"
#if DARWIN  // Mac
#include "include/wrapper/cef_library_loader.h"
#endif

#include <warn/pop>

namespace inviwo {

/*
 * Enable suppport for showing web pages using CEF
 */
class IVW_MODULE_WEBBROWSER_API WebBrowserModule : public InviwoModule {
public:
    WebBrowserModule(InviwoApplication* app);
    virtual ~WebBrowserModule();

    static std::string getDataURI(const std::string& data, const std::string& mime_type);

    // Return error code enum as string
    static std::string getCefErrorString(cef_errorcode_t code);

protected:
    Timer doChromiumWork_;  /// Calls CefDoMessageLoopWork()
#ifdef DARWIN               // Load library dynamically for Mac
    CefScopedLibraryLoader cefLib_;
#endif
};

}  // namespace inviwo

#endif  // IVW_WEBBROWSERMODULE_H
