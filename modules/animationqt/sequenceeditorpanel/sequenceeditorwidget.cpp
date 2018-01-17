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

#include <modules/animationqt/sequenceeditorpanel/sequenceeditorwidget.h>
#include <modules/animationqt/sequenceeditorpanel/sequenceeditorpanel.h>
#include <modules/animationqt/sequenceeditorpanel/keyframeeditorwidget.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <warn/pop>

/**
 * TODO:
 * * Listen to updates to easing from outside
 */

namespace inviwo {

namespace animation {
SequenceEditorWidget::SequenceEditorWidget(KeyframeSequence& sequence, Track& track,
                                           SequenceEditorPanel* panel)
    : QWidget(panel), sequence_(sequence), track_(track) {
    setObjectName("SequenceEditorWidget");
    sequence_.addObserver(this);

    auto layout = new QVBoxLayout();
    setLayout(layout);

    // Sequence (track) Name
    auto label = new QLabel(track_.getName().c_str());
    auto font = label->font();
    font.setBold(true);
    label->setFont(font);
    layout->addWidget(label);

    keyframesLayout_ = new QVBoxLayout();
    layout->addLayout(keyframesLayout_);

	if (dynamic_cast<ControlTrack*>(&track) == nullptr) {
		auto easingLayout = new QHBoxLayout();
		layout->addLayout(easingLayout);

		auto easing = new QComboBox();
		easingLayout->addWidget(new QLabel("Easing: "));
		easingLayout->addWidget(easing);

		auto currentEasing = sequence_.getEasingType();

		for (auto e = easing::FirstEasingType; e <= easing::LastEasingType; ++e) {
			std::ostringstream oss;
			oss << e;
			easing->addItem(oss.str().c_str(), QVariant((int)e));
			if (currentEasing == e) {
				easing->setCurrentIndex(easing->count() - 1);
			}
		}

		void (QComboBox::*signal)(int) = &QComboBox::currentIndexChanged;
		connect(easing, signal,
			[this](int index) { sequence_.setEasingType(static_cast<easing::EasingType>(index)); });
	}

    for (size_t i = 0; i < sequence_.size(); i++) {
        onKeyframeAdded(&sequence_[i], &sequence_);
    }

    updateVisibility();
}

void SequenceEditorWidget::updateVisibility() {
    setVisible(sequence_.isSelected() || sequence_.isAnyKeyframeSelected());
}

void SequenceEditorWidget::onKeyframeSequenceSelectionChanged(KeyframeSequence* seq) {
    updateVisibility();
}

void SequenceEditorWidget::onKeyframeAdded(Keyframe* key, KeyframeSequence* seq) {
    auto w = new KeyframeEditorWidget(*key, this);
    keyframesLayout_->addWidget(w);
    keyframeEditorWidgets_[key] = w;
    setReorderNeeded();
}

void SequenceEditorWidget::onKeyframeRemoved(Keyframe* key, KeyframeSequence* seq) {
    auto widget = keyframeEditorWidgets_[key];
    keyframeEditorWidgets_.erase(key);
    keyframesLayout_->removeWidget(widget);
    setReorderNeeded();
}

void SequenceEditorWidget::setReorderNeeded() { reorderNeeded_ = true; }

void SequenceEditorWidget::reorderKeyframes() {

    std::vector<KeyframeEditorWidget*> widgets(keyframeEditorWidgets_.size());
    std::transform(keyframeEditorWidgets_.begin(), keyframeEditorWidgets_.end(), widgets.begin(),
                   [](auto pair) { return pair.second; });

    std::stable_sort(widgets.begin(), widgets.end(), [](auto a, auto b) {
        return a->getKeyframe().getTime().count() < b->getKeyframe().getTime().count();
    });

    bool orderChanged = false;
    size_t i = 0;
    for (auto w : widgets) {
        orderChanged |= keyframesLayout_->indexOf(w) != i++;
    }

    if (orderChanged) {
        for (auto w : widgets) {
            keyframesLayout_->removeWidget(w);
        }

        for (auto w : widgets) {
            keyframesLayout_->addWidget(w);
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
