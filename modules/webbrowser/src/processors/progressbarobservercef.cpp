/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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

#include <modules/webbrowser/processors/progressbarobservercef.h>

#include <ostream>  // for operator<<, basic_ostream, stringstream

#include <include/base/cef_scoped_refptr.h>  // for scoped_refptr
#include <include/cef_base.h>                // for CefRefPtr
#include <include/cef_frame.h>               // for CefFrame

namespace inviwo {

ProgressBarObserverCEF::ProgressBarObserverCEF(CefRefPtr<CefFrame> frame,
                                               std::string onProgressChange,
                                               std::string onVisibleChange)
    : onProgressChange_(onProgressChange)
    , onProgressVisibleChange_(onVisibleChange)
    , frame_(frame) {}

void ProgressBarObserverCEF::progressChanged(float progress) {
    // Frame might be null if for example webpage is not found on startup
    if (!frame_ || getOnProgressChange().empty()) {
        return;
    }
    std::stringstream script;
    script << getOnProgressChange() << "(" << progress << ");";
    frame_->ExecuteJavaScript(script.str(), frame_->GetURL(), 0);
}

void ProgressBarObserverCEF::progressBarVisibilityChanged(bool visible) {
    // Frame might be null if for example webpage is not found on startup
    if (!frame_ || getOnProgressVisibleChange().empty()) {
        return;
    }
    std::stringstream script;
    script << getOnProgressVisibleChange() << "(" << (visible ? "true" : "false") << ");";
    frame_->ExecuteJavaScript(script.str(), frame_->GetURL(), 0);
}

void ProgressBarObserverCEF::setFrame(CefRefPtr<CefFrame> frame) { frame_ = frame; }

}  // namespace inviwo
