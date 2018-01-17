/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017 Inviwo Foundation
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

#include <modules/animationqt/sequenceeditorpanel/sequenceeditorpanel.h>
#include <modules/animationqt/sequenceeditorpanel/sequenceeditorwidget.h>

#include <modules/animation/datastructures/animation.h>
#include <modules/animation/datastructures/track.h>
#include <modules/animation/datastructures/keyframesequence.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QVBoxLayout>
#include <QScrollArea>
#include <warn/pop>

namespace inviwo {

namespace animation {
SequenceEditorPanel::SequenceEditorPanel(AnimationController& controller, QWidget* parent)
    : QScrollArea(parent), controller_(controller) {
    setObjectName("SequenceEditorPanel");

    // auto scrollArea = new QScrollArea(this);
    auto scrollArea = this;
    scrollArea->setWidgetResizable(true);
    // scrollArea->setMinimumWidth(320);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
#ifdef __APPLE__
    // Scrollbars are overlayed in different way on mac...
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
#else
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
#endif
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setContentsMargins(0, 7, 0, /*PropertyWidgetQt::spacing*/ 7);

    auto* widget = new QWidget();

    auto layout = new QVBoxLayout();
    widget->setLayout(layout);
    layout->setAlignment(Qt::AlignTop);

    sequenceEditors_ = new QVBoxLayout();
    layout->addLayout(sequenceEditors_);

    scrollArea->setWidget(widget);

    if (auto ani = controller.getAnimation()) {
		onTrackAdded(&ani->getControlTrack());
        for (size_t i = 0; i < ani->size(); i++) {
            onTrackAdded(&(*ani)[i]);
        }
        ani->addObserver(this);
    }
}

void SequenceEditorPanel::onAnimationChanged(AnimationController* controller, Animation* oldAnim,
                                             Animation* newAnim) {
    oldAnim->removeObserver(this);

    for (size_t i = 0; i < newAnim->size(); i++) {
        onTrackAdded(&(*newAnim)[i]);
    }
    newAnim->addObserver(this);
}

void SequenceEditorPanel::onTrackAdded(Track* track) {
    for (size_t i = 0; i < track->size(); i++) {
        onKeyframeSequenceAdded(track, &(*track)[i]);
    }
    track->addObserver(this);
}

void SequenceEditorPanel::onTrackRemoved(Track* track) { track->removeObserver(this); }

void SequenceEditorPanel::onKeyframeSequenceAdded(Track* t, KeyframeSequence* s) {
    auto widget = new SequenceEditorWidget(*s, *t, this);
    widgets_[s] = widget;
    sequenceEditors_->addWidget(widget);
}

void SequenceEditorPanel::onKeyframeSequenceRemoved(Track* t, KeyframeSequence* s) {
    auto it = widgets_.find(s);
    if (it != widgets_.end()) {
        sequenceEditors_->removeWidget(it->second);
        delete it->second;
        widgets_.erase(it);
    }
}

}  // namespace animation

}  // namespace inviwo
