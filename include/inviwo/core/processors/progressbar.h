/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#ifndef IVW_PROGRESSBAR_H
#define IVW_PROGRESSBAR_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/observer.h>

namespace inviwo {


/** \class ProgressBarObserver
 *
 * Observer for ProgressBar
 *
 * @see ProgressBar
 * @see ProgressBarObservable
 */
class IVW_CORE_API ProgressBarObserver: public Observer {
public:
    ProgressBarObserver(): Observer() {};

    /**
    * This method will be called when observed object changes.
    * Override it to add behavior.
    */
    virtual void progressChanged() {};

    /**
    * This method will be called when observed object changes.
    * Override it to add behavior.
    */
    virtual void progressBarVisibilityChanged() {};
};


/** \class ProgressBarObservable
 *
 * Observable for ProgressBar.
 *
 * @see ProgressBar
 * @see ProgressBarObserver
 */
class IVW_CORE_API ProgressBarObservable: public Observable<ProgressBarObserver> {
public:
    ProgressBarObservable(): Observable<ProgressBarObserver>() {};

    void notifyProgressChanged() const {
        // Notify observers
        ObserverSet::iterator endIt = observers_->end();

        for (ObserverSet::iterator it = observers_->begin(); it != endIt; ++it) {
            // static_cast can be used since only template class objects can be added
            static_cast<ProgressBarObserver*>(*it)->progressChanged();
        }
    }

    void notifyVisibilityChanged() const {
        // Notify observers
        ObserverSet::iterator endIt = observers_->end();

        for (ObserverSet::iterator it = observers_->begin(); it != endIt; ++it) {
            // static_cast can be used since only template class objects can be added
            static_cast<ProgressBarObserver*>(*it)->progressBarVisibilityChanged();
        }
    }
};

/** \class ProgressBar
 *
 * Simple progressbar to be used in a ProgressBarOwner.
 *
 * @note Use ProgressBarOwner when using it for a Processor
 * @see ProgressBarOwner
 * @see ProgressBarObserver
 */
class IVW_CORE_API ProgressBar : public IvwSerializable, public ProgressBarObservable {

public:
    ProgressBar();
    virtual ~ProgressBar();

    float getProgress() const { return progress_; }
    void resetProgress() { progress_ = 0.0f; notifyProgressChanged(); }
    void finishProgress() { progress_ = 1.0f; notifyProgressChanged(); }

    void updateProgress(float progress);
    void updateProgressLoop(size_t loopVar, size_t maxLoopVar, float endProgress);

    void show() { visible_ = true; notifyVisibilityChanged(); }
    void hide() { visible_ = false; notifyVisibilityChanged(); }
    bool isVisible() const { return visible_; }

    virtual void serialize(IvwSerializer& s) const;
    virtual void deserialize(IvwDeserializer& d);

private:
    float progress_;
    float beginLoopProgress_;
    bool visible_;

};

} // namespace

#endif // IVW_PROGRESSBAR_H
