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

#include <modules/animationqt/sequenceeditor/sequenceeditorwidget.h>

#include <modules/animation/datastructures/keyframesequence.h>
#include <modules/animation/datastructures/track.h>

#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/util/stringconversion.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <warn/pop>

namespace inviwo {

namespace animation {
SequenceEditorWidget::SequenceEditorWidget(KeyframeSequence& sequence, Track& track)
    : QWidget(), sequence_(sequence), track_{track} {
    setObjectName("SequenceEditorWidget");
}

void SequenceEditorWidget::updateVisibility() {
    setVisible(sequence_.isSelected() || sequence_.isAnyKeyframeSelected());
}

void SequenceEditorWidget::onKeyframeSequenceSelectionChanged(KeyframeSequence*) {
    updateVisibility();
}

void SequenceEditorWidget::onKeyframeAdded(Keyframe* key, KeyframeSequence*) {
    auto w = create(key);
    keyframesLayout_->addWidget(w);
    keyframeEditorWidgets_[key] = w;
    setReorderNeeded();
}

void SequenceEditorWidget::onKeyframeRemoved(Keyframe* key, KeyframeSequence*) {
    auto it = keyframeEditorWidgets_.find(key);
    if (it != keyframeEditorWidgets_.end()) {
        keyframesLayout_->removeWidget(it->second);
        delete it->second;
        keyframeEditorWidgets_.erase(it);
        setReorderNeeded();
    }
}

void SequenceEditorWidget::setReorderNeeded() { reorderNeeded_ = true; }

void SequenceEditorWidget::reorderKeyframes() {
    std::vector<std::pair<const Keyframe*, QWidget*>> widgets(keyframeEditorWidgets_.begin(),
                                                              keyframeEditorWidgets_.end());

    std::stable_sort(widgets.begin(), widgets.end(), [](auto& a, auto& b) {
        return a.first->getTime().count() < b.first->getTime().count();
    });

    bool orderChanged = false;
    int i = 0;
    for (auto w : widgets) {
        orderChanged |= keyframesLayout_->indexOf(w.second) != i++;
    }

    if (orderChanged) {
        for (auto w : widgets) {
            keyframesLayout_->removeWidget(w.second);
        }
        for (auto w : widgets) {
            keyframesLayout_->addWidget(w.second);
        }
    };
}

void SequenceEditorWidget::paintEvent(QPaintEvent* event) {
    if (reorderNeeded_) {
        reorderKeyframes();
        reorderNeeded_ = false;
    }

    QWidget::paintEvent(event);
}

}  // namespace animation

}  // namespace inviwo
