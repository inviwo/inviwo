/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifndef IVW_TRACKOBSERVER_H
#define IVW_TRACKOBSERVER_H

#include <modules/animation/animationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/observer.h>

namespace inviwo {

namespace animation {

class Track;
class KeyframeSequence;

class IVW_MODULE_ANIMATION_API TrackObserver : public Observer {
public:
    virtual void onKeyframeSequenceAdded(Track*, KeyframeSequence*){};
    virtual void onKeyframeSequenceRemoved(Track*, KeyframeSequence*){};

    virtual void onFirstMoved(Track*){};
    virtual void onLastMoved(Track*){};

    virtual void onEnabledChanged(Track*){};
    virtual void onIdentifierChanged(Track*){};
    virtual void onNameChanged(Track*){};
    virtual void onPriorityChanged(Track*){};
};

class IVW_MODULE_ANIMATION_API TrackObservable : public Observable<TrackObserver> {
protected:
    void notifyKeyframeSequenceAdded(Track* t, KeyframeSequence* s);
    void notifyKeyframeSequenceRemoved(Track* t, KeyframeSequence* s);

    void notifyFirstMoved(Track* t);
    void notifyLastMoved(Track* t);

    void notifyEnabledChanged(Track* t);
    void notifyIdentifierChanged(Track* t);
    void notifyNameChanged(Track* t);
    void notifyPriorityChanged(Track* t);
};

}  // namespace animation

}  // namespace inviwo

#endif  // IVW_TRACKOBSERVER_H
