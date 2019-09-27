/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/progressbar.h>

#include <warn/push>
#include <warn/ignore/all>
#include <include/cef_frame.h>
#include <warn/pop>
namespace inviwo {

/**
 * \brief Observes progress bar changes and notifies supplied javascript functions on change.
 */
class IVW_MODULE_WEBBROWSER_API ProgressBarObserverCEF : public ProgressBarObserver {
public:
    ProgressBarObserverCEF(CefRefPtr<CefFrame> frame = nullptr, std::string onProgressChange = "",
                           std::string onVisibleChange = "");
    virtual ~ProgressBarObserverCEF(){};

    /**
     * Execute currently set onProgressChange javascript function
     * @param New progress between [0 1]
     */
    virtual void progressChanged(float progress) override;

    /**
     * Execute currently set OnProgressVisibleChange javascript function
     * @param visibility state that ProgressBar changed into
     */
    virtual void progressBarVisibilityChanged(bool visible) override;
    /*
     * Name of javascript function to call when progress changes
     */
    void setOnProgressChange(std::string onChange) { onProgressChange_ = onChange; }
    const std::string& getOnProgressChange() const { return onProgressChange_; }
    /*
     * Name of javascript function to call when progress bar visibility changes
     */
    void setOnProgressVisibleChange(std::string onChange) { onProgressVisibleChange_ = onChange; }
    const std::string& getOnProgressVisibleChange() const { return onProgressVisibleChange_; }

    /*
     * Set frame containing html item.
     */
    void setFrame(CefRefPtr<CefFrame> frame);

private:
    std::string onProgressChange_;         /// Javascript callback to execute when progress changes
    std::string onProgressVisibleChange_;  /// Javascript callback to execute when vibility changes
    CefRefPtr<CefFrame> frame_;            /// Browser frame containing corresponding callbacks
};

}  // namespace inviwo
