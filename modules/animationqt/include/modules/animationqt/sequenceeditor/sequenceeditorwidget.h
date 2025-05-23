/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2025 Inviwo Foundation
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

#include <modules/animationqt/animationqtmoduledefine.h>  // for IVW_MODULE_ANIMAT...

#include <modules/animation/datastructures/keyframesequenceobserver.h>  // for KeyframeSequenceO...

#include <unordered_map>  // for unordered_map

#include <QWidget>  // for QWidget

class QPaintEvent;
class QVBoxLayout;

namespace inviwo {

namespace animation {

class AnimationManager;
class Keyframe;
class KeyframeSequence;
class Track;

class IVW_MODULE_ANIMATIONQT_API SequenceEditorWidget : public QWidget,
                                                        public KeyframeSequenceObserver {
public:
    SequenceEditorWidget(KeyframeSequence& sequence, Track& track, AnimationManager& manager);
    virtual ~SequenceEditorWidget() = default;

    void updateVisibility();

    // KeyframeSequenceObserver overloads
    virtual void onKeyframeSequenceSelectionChanged(KeyframeSequence* seq) override;
    virtual void onKeyframeAdded(Keyframe* key, KeyframeSequence* seq) override;
    virtual void onKeyframeRemoved(Keyframe* key, KeyframeSequence* seq) override;

    Track& getTrack() { return track_; }

    void setReorderNeeded();

protected:
    virtual QWidget* create(Keyframe* key) = 0;
    void reorderKeyframes();
    virtual void paintEvent(QPaintEvent* event) override;

    KeyframeSequence& sequence_;
    Track& track_;

    std::unordered_map<Keyframe*, QWidget*> keyframeEditorWidgets_;

    QVBoxLayout* keyframesLayout_{nullptr};

    bool reorderNeeded_{true};
};

}  // namespace animation

}  // namespace inviwo
