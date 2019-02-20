/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#ifndef IVW_SEQUENCEEDITORPANEL_H
#define IVW_SEQUENCEEDITORPANEL_H

#include <modules/animationqt/animationqtmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <modules/animation/animationcontroller.h>
#include <modules/animation/animationcontrollerobserver.h>
#include <modules/animation/datastructures/animationobserver.h>
#include <modules/animation/datastructures/trackobserver.h>

#include <modules/qtwidgets/inviwodockwidget.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QWidget>
#include <QScrollArea>
#include <warn/pop>

class QVBoxLayout;

namespace inviwo {

namespace animation {

class AnimationManager;
class SequenceEditorWidget;
class SequenceEditorFactory;
class EditorWidgetFactory;

class IVW_MODULE_ANIMATIONQT_API SequenceEditorPanel : public QScrollArea,
                                                       public AnimationControllerObserver,
                                                       public AnimationObserver,
                                                       public TrackObserver {
public:
    SequenceEditorPanel(AnimationManager& manager, SequenceEditorFactory& editorFactory,
                        QWidget* parent = nullptr);
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

#endif  // IVW_SEQUENCEEDITORPANEL_H
