/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2025 Inviwo Foundation
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

#include <inviwo/core/util/logcentral.h>

#include <tuple>  // for tuple

#include <warn/push>
#include <warn/ignore/all>
#include <include/cef_base.h>  // for CefBrowserSettings

#include <warn/pop>

namespace inviwo::cefutil {

/**
 * Get default settings for CefBrowserHost::CreateBrowserSync
 *
 * Enable loading files from other locations than where the .html file is
 * browserSettings.file_access_from_file_urls = STATE_ENABLED;
 *
 * window_info.SetAsWindowless(nullptr);  // nullptr means no transparency (site background colour)
 */
IVW_MODULE_WEBBROWSER_API std::tuple<CefWindowInfo, CefBrowserSettings> getDefaultBrowserSettings();

// CEF uses a zoom level which increases/decreases by 20% per level
//
// see https://bugs.chromium.org/p/chromium/issues/detail?id=71484

IVW_MODULE_WEBBROWSER_API double percentageToZoomLevel(double percent);

IVW_MODULE_WEBBROWSER_API double zoomLevelToPercentage(double level);

IVW_MODULE_WEBBROWSER_API LogLevel logLevel(cef_log_severity_t level);

IVW_MODULE_WEBBROWSER_API std::string getDataURI(const std::string& data,
                                                 const std::string& mime_type);

// Return error code enum as string
IVW_MODULE_WEBBROWSER_API std::string getCefErrorString(cef_errorcode_t code);

}  // namespace inviwo::cefutil
