/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2025 Inviwo Foundation
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

#include <modules/animationqt/sequenceeditor/controlsequenceeditor.h>

#include <modules/animation/datastructures/animationtime.h>           // for Seconds
#include <modules/animation/datastructures/controlkeyframe.h>         // for ControlKeyframe
#include <modules/animation/datastructures/keyframe.h>                // for Keyframe
#include <modules/animation/datastructures/keyframeobserver.h>        // for KeyframeObserver
#include <modules/animation/datastructures/keyframesequence.h>        // for KeyframeSequence
#include <modules/animation/datastructures/track.h>                   // for Track
#include <modules/animationqt/sequenceeditor/sequenceeditorwidget.h>  // for SequenceEditorWidget
#include <modules/qtwidgets/inviwoqtutils.h>                          // for toQString
#include <modules/qtwidgets/properties/doublevaluedragspinbox.h>

#include <cstddef>  // for size_t

#include <QComboBox>    // for QComboBox
#include <QFont>        // for QFont
#include <QHBoxLayout>  // for QHBoxLayout
#include <QLabel>       // for QLabel
#include <QVBoxLayout>  // for QVBoxLayout
#include <QWidget>      // for QWidget

namespace inviwo {

namespace animation {

namespace {

class ControlEditorWidget : public QWidget, public KeyframeObserver {
public:
    ControlEditorWidget(Keyframe& keyframe, SequenceEditorWidget* parent)
        : QWidget(parent), keyframe_(keyframe), sequenceEditorWidget_(parent) {

        auto& ctrlKey = dynamic_cast<ControlKeyframe&>(keyframe);

        setObjectName("KeyframeEditorWidget");
        setVisible(keyframe_.isSelected());
        keyframe.addObserver(this);

        auto layout = new QHBoxLayout();
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(7);

        timeSpinner_ = new DoubleValueDragSpinBox();
        timeSpinner_->setValue(keyframe.getTime().count());
        timeSpinner_->setSingleStep(0.1);
        timeSpinner_->setDecimals(3);
        timeSpinner_->setMaximum(10000.0);
        timeSpinner_->setToolTip("Time [s]");

        connect(timeSpinner_,
                static_cast<void (DoubleValueDragSpinBox::*)(double)>(
                    &DoubleValueDragSpinBox::valueChanged),
                this, [this](double t) { keyframe_.setTime(Seconds(t)); });
        connect(timeSpinner_, &DoubleValueDragSpinBox::editingFinished, this,
                [this]() { keyframe_.setTime(Seconds(timeSpinner_->value())); });

        layout->addWidget(timeSpinner_);

        actionWidget_ = new QComboBox();
        actionWidget_->addItems({"Pause", "Jump To"});
        actionWidget_->setCurrentIndex(static_cast<int>(ctrlKey.getAction()));

        jumpToWidget_ = new DoubleValueDragSpinBox();
        jumpToWidget_->setValue(ctrlKey.getJumpTime().count());
        jumpToWidget_->setSingleStep(0.1);
        jumpToWidget_->setDecimals(3);
        jumpToWidget_->setVisible(ctrlKey.getAction() == ControlAction::Jump);
        jumpToWidget_->setMaximum(10000.0);
        jumpToWidget_->setToolTip("Time [s]");

        connect(actionWidget_, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this,
                [this, &ctrlKey](int idx) {
                    ctrlKey.setAction(static_cast<ControlAction>(idx));
                    jumpToWidget_->setVisible(ctrlKey.getAction() == ControlAction::Jump);
                });

        connect(jumpToWidget_,
                static_cast<void (DoubleValueDragSpinBox::*)(double)>(
                    &DoubleValueDragSpinBox::valueChanged),
                this, [&ctrlKey](double t) { ctrlKey.setJumpTime(Seconds{t}); });
        connect(jumpToWidget_, &DoubleValueDragSpinBox::editingFinished, this,
                [&]() { ctrlKey.setJumpTime(Seconds{jumpToWidget_->value()}); });

        layout->addWidget(actionWidget_);
        layout->addWidget(jumpToWidget_);

        setLayout(layout);
    }
    virtual ~ControlEditorWidget() = default;

    virtual void onKeyframeTimeChanged(Keyframe* key, Seconds) override {
        timeSpinner_->setValue(key->getTime().count());
        sequenceEditorWidget_->setReorderNeeded();
    }

    Keyframe& getKeyframe() { return keyframe_; }

    virtual void onKeyframeSelectionChanged(Keyframe* key) override {
        setVisible(key->isSelected());
        sequenceEditorWidget_->updateVisibility();
    }

private:
    Keyframe& keyframe_;
    SequenceEditorWidget* sequenceEditorWidget_{nullptr};

    QComboBox* actionWidget_{nullptr};
    DoubleValueDragSpinBox* jumpToWidget_{nullptr};
    DoubleValueDragSpinBox* timeSpinner_{nullptr};
};

}  // namespace

ControlSequenceEditor::ControlSequenceEditor(KeyframeSequence& sequence, Track& track,
                                             AnimationManager& manager)
    : SequenceEditorWidget(sequence, track, manager) {

    sequence_.addObserver(this);

    setContentsMargins(7, 7, 0, 7);

    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(7);
    setLayout(layout);

    auto label = new QLabel(utilqt::toQString(track_.getName()));
    auto font = label->font();
    font.setBold(true);
    label->setFont(font);
    layout->addWidget(label);

    keyframesLayout_ = new QVBoxLayout();
    keyframesLayout_->setContentsMargins(0, 0, 0, 0);
    keyframesLayout_->setSpacing(7);
    layout->addLayout(keyframesLayout_);

    for (size_t i = 0; i < sequence_.size(); i++) {
        onKeyframeAdded(&sequence_[i], &sequence_);
    }

    updateVisibility();
}

QWidget* ControlSequenceEditor::create(Keyframe* key) {
    return new ControlEditorWidget(*key, this);
}

std::string ControlSequenceEditor::classIdentifier() {
    return "org.inviwo.animation.ControlSequenceEditor";
}

}  // namespace animation

}  // namespace inviwo
