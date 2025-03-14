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

#include <modules/animationqt/sequenceeditor/sequenceeditorpanel.h>

#include <inviwo/core/util/indirectiterator.h>                        // for IndirectIterator
#include <modules/animation/animationcontroller.h>                    // for AnimationController
#include <modules/animation/animationcontrollerobserver.h>            // for AnimationController...
#include <modules/animation/datastructures/animation.h>               // for Animation
#include <modules/animation/datastructures/track.h>                   // for Track
#include <modules/animationqt/factories/sequenceeditorfactory.h>      // for SequenceEditorFactory
#include <modules/animationqt/sequenceeditor/sequenceeditorwidget.h>  // for SequenceEditorWidget

#include <cstddef>  // for size_t
#include <memory>   // for unique_ptr
#include <string>   // for basic_string, string
#include <utility>  // for pair

#include <QFrame>       // for QFrame, QFrame::NoF...
#include <QScrollArea>  // for QScrollArea
#include <QVBoxLayout>  // for QVBoxLayout
#include <QWidget>      // for QWidget
#include <Qt>           // for ScrollBarAsNeeded

class QLayout;

namespace inviwo {

namespace animation {

class KeyframeSequence;

SequenceEditorPanel::SequenceEditorPanel(AnimationController& controller, AnimationManager& manager,
                                         SequenceEditorFactory& editorFactory, QWidget* parent)
    : QScrollArea(parent), manager_(manager), factory_{editorFactory} {
    setObjectName("SequenceEditorPanel");

    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
#ifdef __APPLE__
    // Scrollbars are overlayed in different way on mac...
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
#else
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
#endif
    setFrameShape(QFrame::NoFrame);

    setContentsMargins(0, 0, 0, 0);

    auto upper = new QWidget();
    sequenceEditors_ = new QVBoxLayout();
    upper->setLayout(sequenceEditors_);
    sequenceEditors_->setAlignment(Qt::AlignTop);
    sequenceEditors_->setContentsMargins(7, 7, 0, 7);
    sequenceEditors_->setSpacing(7);

    auto lower = new QWidget();
    optionLayout_ = new QVBoxLayout();
    lower->setLayout(optionLayout_);
    optionLayout_->setAlignment(Qt::AlignBottom);
    optionLayout_->setContentsMargins(7, 7, 0, 7);
    optionLayout_->setSpacing(7);

    auto widget = new QWidget();
    auto baseLayout = new QVBoxLayout();
    widget->setLayout(baseLayout);
    baseLayout->setContentsMargins(0, 0, 0, 0);
    baseLayout->setSpacing(0);
    baseLayout->addWidget(upper);
    baseLayout->addWidget(lower);
    setWidget(widget);

    for (auto& track : controller.getAnimation()) {
        onTrackAdded(&track);
    }
    controller.getAnimation().addObserver(this);
    controller.AnimationControllerObservable::addObserver(this);
}

QLayout* SequenceEditorPanel::getOptionLayout() { return optionLayout_; }

void SequenceEditorPanel::onAnimationChanged(AnimationController*, Animation* oldAnim,
                                             Animation* newAnim) {
    oldAnim->removeObserver(this);
    while (!widgets_.empty()) {
        onTrackRemoved(&widgets_.begin()->second->getTrack());
    }
    for (auto& track : *newAnim) {
        onTrackAdded(&track);
    }
    newAnim->addObserver(this);
}

void SequenceEditorPanel::onTrackAdded(Track* track) {
    for (size_t i = 0; i < track->size(); i++) {
        onKeyframeSequenceAdded(track, &(*track)[i]);
    }
    track->addObserver(this);
}

void SequenceEditorPanel::onTrackRemoved(Track* track) {
    for (size_t i = 0; i < track->size(); i++) {
        onKeyframeSequenceRemoved(track, &(*track)[i]);
    }
    track->removeObserver(this);
}

void SequenceEditorPanel::onKeyframeSequenceAdded(Track* t, KeyframeSequence* s) {
    auto widgetId = factory_.getSequenceEditorId(t->getClassIdentifier());
    if (auto widget = factory_.create(widgetId, *s, *t, manager_)) {
        widgets_[s] = widget.get();
        sequenceEditors_->addWidget(widget.release());
    }
}

void SequenceEditorPanel::onKeyframeSequenceRemoved(Track*, KeyframeSequence* s) {
    auto it = widgets_.find(s);
    if (it != widgets_.end()) {
        sequenceEditors_->removeWidget(it->second);
        delete it->second;
        widgets_.erase(it);
    }
}

}  // namespace animation

}  // namespace inviwo
