/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2018 Inviwo Foundation
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

#ifndef IVW_KEYFRAMESEQUENCEOBSERVER_H
#define IVW_KEYFRAMESEQUENCEOBSERVER_H

#include <modules/animation/animationmoduledefine.h>
#include <inviwo/core/util/observer.h>

namespace inviwo {

namespace animation {

class Keyframe;
class KeyframeSequence;

class IVW_MODULE_ANIMATION_API KeyframeSequenceObserver : public Observer {
public:
    virtual void onKeyframeAdded(Keyframe*, KeyframeSequence*){};
    virtual void onKeyframeRemoved(Keyframe*, KeyframeSequence*){};
    virtual void onKeyframeSequenceMoved(KeyframeSequence*){};
    virtual void onKeyframeSequenceSelectionChanged(KeyframeSequence*){};
};

class IVW_MODULE_ANIMATION_API KeyframeSequenceObserverble
    : public Observable<KeyframeSequenceObserver> {
protected:
    void notifyKeyframeAdded(Keyframe* key, KeyframeSequence* seq);
    void notifyKeyframeRemoved(Keyframe* key, KeyframeSequence* seq);
    void notifyKeyframeSequenceMoved(KeyframeSequence* seq);
    void notifyKeyframeSequenceSelectionChanged(KeyframeSequence* seq);
};

}  // namespace animation

}  // namespace inviwo

#endif  // IVW_KEYFRAMESEQUENCEOBSERVER_H
