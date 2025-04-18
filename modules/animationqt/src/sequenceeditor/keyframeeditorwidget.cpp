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

#include <modules/animationqt/sequenceeditor/keyframeeditorwidget.h>

#include <inviwo/core/common/factoryutil.h>                           // for getPropertyWidgetFa...
#include <inviwo/core/properties/property.h>                          // for Property
#include <inviwo/core/properties/propertywidgetfactory.h>             // for PropertyWidgetFactory
#include <modules/animation/datastructures/animationtime.h>           // for Seconds
#include <modules/animation/datastructures/controlkeyframe.h>         // for ControlKeyframe
#include <modules/animation/datastructures/keyframe.h>                // for Keyframe
#include <modules/animation/datastructures/propertytrack.h>           // for BasePropertyTrack
#include <modules/animation/datastructures/track.h>                   // for Track
#include <modules/animationqt/sequenceeditor/sequenceeditorwidget.h>  // for SequenceEditorWidget
#include <modules/qtwidgets/editablelabelqt.h>                        // for EditableLabelQt
#include <modules/qtwidgets/properties/propertywidgetqt.h>            // for PropertyWidgetQt
#include <modules/qtwidgets/properties/doublevaluedragspinbox.h>

#include <functional>  // for __base

#include <QComboBox>    // for QComboBox
#include <QHBoxLayout>  // for QHBoxLayout

namespace inviwo {

namespace animation {
KeyframeEditorWidget::KeyframeEditorWidget(Keyframe& keyframe, SequenceEditorWidget* parent)
    : QWidget(parent), keyframe_(keyframe), sequenceEditorWidget_(parent) {

    setObjectName("KeyframeEditorWidget");

    keyframe.addObserver(this);

    layout_ = new QHBoxLayout();

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

    layout_->addWidget(timeSpinner_);

    if (auto propTrack = dynamic_cast<BasePropertyTrack*>(&parent->getTrack())) {
        auto baseProperty = propTrack->getProperty();
        property_.reset(baseProperty->clone());
        propTrack->setPropertyFromKeyframe(property_.get(), &keyframe);
        property_->onChange([p = property_.get(), t = propTrack, k = &keyframe_]() {
            t->setKeyframeFromProperty(p, k);
        });
        property_->setOwner(nullptr);

        auto propWidget = util::getPropertyWidgetFactory()->create(property_.get());
        propertyWidget_ = static_cast<PropertyWidgetQt*>(propWidget.release());

        if (auto label = propertyWidget_->findChild<EditableLabelQt*>()) {
            label->setVisible(false);
        }
        layout_->addWidget(propertyWidget_);

    } else if (auto ctrlKey = dynamic_cast<ControlKeyframe*>(&keyframe)) {

        actionWidget_ = new QComboBox();
        actionWidget_->addItems({"Pause", "Jump To"});
        actionWidget_->setCurrentIndex(static_cast<int>(ctrlKey->getAction()));

        jumpToWidget_ = new DoubleValueDragSpinBox();
        jumpToWidget_->setValue(ctrlKey->getJumpTime().count());
        jumpToWidget_->setSingleStep(0.1);
        jumpToWidget_->setDecimals(3);
        jumpToWidget_->setMaximum(10000.0);
        jumpToWidget_->setToolTip("Time [s]");
        jumpToWidget_->setVisible(ctrlKey->getAction() == ControlAction::Jump);

        connect(actionWidget_, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this,
                [this, ctrlKey](int idx) {
                    ctrlKey->setAction(static_cast<ControlAction>(idx));
                    jumpToWidget_->setVisible(ctrlKey->getAction() == ControlAction::Jump);
                });

        connect(jumpToWidget_,
                static_cast<void (DoubleValueDragSpinBox::*)(double)>(
                    &DoubleValueDragSpinBox::valueChanged),
                this, [ctrlKey](double t) { ctrlKey->setJumpTime(Seconds{t}); });
        connect(jumpToWidget_, &DoubleValueDragSpinBox::editingFinished, this,
                [&]() { ctrlKey->setJumpTime(Seconds{jumpToWidget_->value()}); });

        layout_->addWidget(actionWidget_);
        layout_->addWidget(jumpToWidget_);
    }

    setLayout(layout_);
}

KeyframeEditorWidget::~KeyframeEditorWidget() {
    if (propertyWidget_) {
        // We need to manually remove and delete the propertyWidget_ since the destructor of
        // propertyWidget_ tries to use the property. If we do not remove it it will be destroyed in
        // QWidgets destructor which is called after this destructor, hence, the property has been
        // destroyed.
        layout_->removeWidget(propertyWidget_);
        delete propertyWidget_;
    }
}

void KeyframeEditorWidget::onKeyframeTimeChanged(Keyframe* key, Seconds) {
    timeSpinner_->setValue(key->getTime().count());
    sequenceEditorWidget_->setReorderNeeded();
}

void KeyframeEditorWidget::onKeyframeSelectionChanged(Keyframe*) {
    sequenceEditorWidget_->updateVisibility();
}

}  // namespace animation

}  // namespace inviwo
