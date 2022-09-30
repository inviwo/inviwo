/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2022 Inviwo Foundation
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

#include <modules/animationqt/animationqtmoduledefine.h>  // for IVW_MODULE_ANIMATIONQT_API

#include <modules/animation/animationcontrollerobserver.h>       // for AnimationControllerObserver
#include <modules/animation/datastructures/animationobserver.h>  // for AnimationObserver
#include <modules/animation/datastructures/trackobserver.h>      // for TrackObserver

#include <unordered_map>  // for unordered_map

#include <warn/push>
#include <warn/ignore/all>
#include <QScrollArea>  // for QScrollArea

class QLayout;
class QWidget;
namespace inviwo {
namespace animation {
class AnimationController;
class KeyframeSequence;
class Track;
}  // namespace animation
}  // namespace inviwo

#include <warn/pop>

class QVBoxLayout;

namespace inviwo {

namespace animation {

class Animation;
class AnimationManager;
class SequenceEditorFactory;
class SequenceEditorWidget;

class IVW_MODULE_ANIMATIONQT_API SequenceEditorPanel : public QScrollArea,
                                                       public AnimationControllerObserver,
                                                       public AnimationObserver,
                                                       public TrackObserver {
public:
    SequenceEditorPanel(AnimationController& controller, AnimationManager& manager,
                        SequenceEditorFactory& editorFactory, QWidget* parent = nullptr);
    virtual ~SequenceEditorPanel() = default;

    virtual void onAnimationChanged(AnimationController* controller, Animation* oldAnim,
                                    Animation* newAnim) override;

    virtual void onTrackAdded(Track* track) override;
    virtual void onTrackRemoved(Track* track) override;

    virtual void onKeyframeSequenceAdded(Track* t, KeyframeSequence* s) override;
    virtual void onKeyframeSequenceRemoved(Track* t, KeyframeSequence* s) override;

    QLayout* getOptionLayout();

private:
    AnimationManager& manager_;
    SequenceEditorFactory& factory_;

    QVBoxLayout* sequenceEditors_{nullptr};
    QVBoxLayout* optionLayout_{nullptr};

    std::unordered_map<KeyframeSequence*, SequenceEditorWidget*> widgets_;
};
}  // namespace animation

}  // namespace inviwo
