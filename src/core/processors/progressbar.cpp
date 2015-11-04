/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

ProgressBar::ProgressBar() : progress_(0.0f), beginLoopProgress_(-1.0f) {}

ProgressBar::~ProgressBar() {}

float ProgressBar::getProgress() const { return progress_; }

void ProgressBar::resetProgress() {
    setActive(false);
    progress_ = 0.0f;
    notifyProgressChanged();
}

void ProgressBar::finishProgress() {
    setActive(false);
    progress_ = 1.0f;
    notifyProgressChanged();
}

void ProgressBar::updateProgress(float progress) {
    setActive(progress>0.0f && progress<1.0f);
    if (visible_) {
        progress_ = progress;
        notifyProgressChanged();
    }
}

void ProgressBar::updateProgressLoop(size_t loopVar, size_t maxLoopVar, float endLoopProgress) {
    if (visible_) {
        if (beginLoopProgress_ <= 0.0f) beginLoopProgress_ = progress_;

        float normalizedLoopVar = static_cast<float>(loopVar) / static_cast<float>(maxLoopVar);
        progress_ = beginLoopProgress_ + normalizedLoopVar * (endLoopProgress - beginLoopProgress_);

        if (loopVar == maxLoopVar) beginLoopProgress_ = -1.0f;

        notifyProgressChanged();
    }
}

void ProgressBar::show() {
    visible_ = true;
    setActive(true);
    notifyVisibilityChanged();
}

void ProgressBar::hide() {
    visible_ = false;
    setActive(false);
    notifyVisibilityChanged();
}

bool ProgressBar::isVisible() const { return visible_; }

void ProgressBar::serialize(Serializer& s) const { s.serialize("visible", visible_); }

void ProgressBar::deserialize(Deserializer& d) {
    d.deserialize("visible", visible_);
    notifyVisibilityChanged();
}

void ProgressBarObservable::notifyProgressChanged() const {
    for (auto o : observers_) o->progressChanged();
}

void ProgressBarObservable::notifyVisibilityChanged() const {
    for (auto o : observers_) o->progressBarVisibilityChanged();
}

}  // namespace
