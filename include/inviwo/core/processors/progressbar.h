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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/observer.h>
#include <inviwo/core/processors/activityindicator.h>

namespace inviwo {

/** \class ProgressBarObserver
 *
 * Observer for ProgressBar
 *
 * @see ProgressBar
 * @see ProgressBarObservable
 */
class IVW_CORE_API ProgressBarObserver : public Observer {
public:
    /**
     * This method will be called when observed object changes.
     * Override it to add behavior.
     * @param New progress between [0 1]
     */
    virtual void progressChanged(float){};

    /**
     * This method will be called when observed object changes.
     * Override it to add behavior.
     * @param visibility state that ProgressBar changed into
     */
    virtual void progressBarVisibilityChanged(bool){};
};

/** \class ProgressBarObservable
 * Observable for ProgressBar.
 * @see ProgressBar
 * @see ProgressBarObserver
 */
class IVW_CORE_API ProgressBarObservable : public Observable<ProgressBarObserver> {
protected:
    void notifyProgressChanged(float progress);
    void notifyVisibilityChanged(bool visible);
};

/** \class ProgressBar
 *
 * Simple progressbar to be used in a ProgressBarOwner.
 * Expects progress between [0 1]. Progress 0 means that it starts and 1 it is finished.
 * @note Use ProgressBarOwner when using it for a Processor
 * @see ProgressBarOwner
 * @see ProgressBarObserver
 */
class IVW_CORE_API ProgressBar : public ActivityIndicator, public ProgressBarObservable {
public:
    ProgressBar();
    virtual ~ProgressBar();
    /**
     * Return current progress. Progress 0 means that it starts and 1 it is finished.
     */
    float getProgress() const;
    /**
     * Set progress to 0 and notify observables if visible.
     */
    void resetProgress();
    /**
     * Set progress to 1 and notify observables if visible.
     */
    void finishProgress();
    /**
     * Set progress in [0 1]. Progress 0 means that it starts and 1 it is finished.
     * Will also notify observables if visible.
     * @param progress between [0 1]
     */
    void updateProgress(float progress);

    void show();
    void hide();
    void setVisible(bool visible);
    bool isVisible() const;

private:
    float progress_;
    bool visible_;
};

}  // namespace inviwo
