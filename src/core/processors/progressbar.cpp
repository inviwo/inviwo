/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#include <inviwo/core/processors/progressbar.h>
#include <inviwo/core/io/serialization/serialization.h>
#include <inviwo/core/util/assertion.h>

namespace inviwo {

ProgressBar::ProgressBar() : progress_(0.0f), visible_{false} {}

ProgressBar::~ProgressBar() = default;

float ProgressBar::getProgress() const { return progress_; }

void ProgressBar::resetProgress() {
    setActive(false);
    if (progress_ != 0.0f) {
        progress_ = 0.0f;
        notifyProgressChanged(progress_);
    }
}

void ProgressBar::finishProgress() {
    setActive(false);
    if (progress_ != 1.0f) {
        progress_ = 1.0f;
        notifyProgressChanged(progress_);
    }
}

void ProgressBar::updateProgress(float progress) {
    setActive(progress < 1.0f);
    if (progress_ != progress) {
        progress_ = progress;
        notifyProgressChanged(progress_);
    }
}

void ProgressBar::show() { setVisible(true); }

void ProgressBar::hide() { setVisible(false); }

void ProgressBar::setVisible(bool visible) {
    setActive(visible);
    if (visible_ != visible) {
        visible_ = visible;
        notifyVisibilityChanged(visible_);
    }
}

bool ProgressBar::isVisible() const { return visible_; }

void ProgressBarObservable::notifyProgressChanged(float progress) {
    forEachObserver([progress](ProgressBarObserver* o) { o->progressChanged(progress); });
}

void ProgressBarObservable::notifyVisibilityChanged(bool visible) {
    forEachObserver(
        [visible](ProgressBarObserver* o) { o->progressBarVisibilityChanged(visible); });
}

}  // namespace inviwo
