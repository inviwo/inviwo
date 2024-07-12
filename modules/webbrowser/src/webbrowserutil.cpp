/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2024 Inviwo Foundation
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

#include <modules/webbrowser/webbrowserutil.h>

#include <tuple>  // for tuple
#include <cmath>

#include <include/cef_base.h>  // for CefBrowserSettings, CefWindowInfo, STATE_ENABLED

namespace inviwo::cefutil {

std::tuple<CefWindowInfo, CefBrowserSettings> getDefaultBrowserSettings() {
    CefWindowInfo windowInfo;

#if defined(WIN32) || defined(__APPLE__)
    windowInfo.SetAsWindowless(nullptr);  // nullptr means no transparency (site background colour)
#else
    windowInfo.SetAsWindowless(0);
#endif

    CefBrowserSettings browserSettings;

    return std::tuple<CefWindowInfo, CefBrowserSettings>{windowInfo, browserSettings};
}

double percentageToZoomLevel(double percent) { return std::log(percent) / std::log(1.2); }

double zoomLevelToPercentage(double level) { return std::pow(1.2, level); }

}  // namespace inviwo::cefutil
