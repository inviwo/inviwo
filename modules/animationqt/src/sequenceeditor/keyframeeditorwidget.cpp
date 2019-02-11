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

#include <modules/animationqt/sequenceeditor/keyframeeditorwidget.h>
#include <modules/animationqt/sequenceeditor/sequenceeditorwidget.h>
#include <modules/animation/datastructures/propertytrack.h>
#include <modules/animation/datastructures/controltrack.h>

#include <modules/qtwidgets/properties/ordinalpropertywidgetqt.h>

#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/propertywidgetfactory.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QtGlobal>
#include <warn/pop>

namespace inviwo {

namespace animation {
KeyframeEditorWidget::KeyframeEditorWidget(Keyframe &keyframe, SequenceEditorWidget *parent)
    : QWidget(parent), keyframe_(keyframe), sequenceEditorWidget_(parent) {

    setObjectName("KeyframeEditorWidget");

    keyframe.addObserver(this);

    layout_ = new QHBoxLayout();

    timeSpinner_ = new QDoubleSpinBox();
    timeSpinner_->setValue(keyframe.getTime().count());
    timeSpinner_->setSuffix("s");
    timeSpinner_->setSingleStep(0.1);
    timeSpinner_->setDecimals(3);

    connect(timeSpinner_,
            static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this,
            [this](double t) { keyframe_.setTime(Seconds(t)); });

    layout_->addWidget(timeSpinner_);

    if (auto propTrack = dynamic_cast<BasePropertyTrack *>(&parent->getTrack())) {
        auto baseProperty = propTrack->getProperty();
        property_.reset(baseProperty->clone());
        propTrack->setOtherProperty(property_.get(), &keyframe);
        property_->onChange([p = property_.get(), t = propTrack, k = &keyframe_]() {
            t->updateKeyframeFromProperty(p, k);
        });
        property_->setOwner(nullptr);

        auto propWidget =
            util::getInviwoApplication()->getPropertyWidgetFactory()->create(property_.get());
        propertyWidget_ = static_cast<PropertyWidgetQt *>(propWidget.release());

        if (auto label = propertyWidget_->findChild<EditableLabelQt *>()) {
            label->setVisible(false);
        }
        layout_->addWidget(propertyWidget_);

    } else if (auto ctrlKey = dynamic_cast<ControlKeyframe *>(&keyframe)) {

        actionWidget_ = new QComboBox();
        actionWidget_->addItems({"Pause", "Jump To"});
        actionWidget_->setCurrentIndex(static_cast<int>(ctrlKey->getAction()));

        jumpToWidget_ = new QDoubleSpinBox();
        jumpToWidget_->setValue(ctrlKey->getJumpTime().count());
        jumpToWidget_->setSuffix("s");
        jumpToWidget_->setSingleStep(0.1);
        jumpToWidget_->setDecimals(3);
        jumpToWidget_->setVisible(ctrlKey->getAction() == ControlAction::Jump);

        connect(actionWidget_, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this,
                [this, ctrlKey](int idx) {
                    ctrlKey->setAction(static_cast<ControlAction>(idx));
                    jumpToWidget_->setVisible(ctrlKey->getAction() == ControlAction::Jump);
                });

        connect(timeSpinner_,
                static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this,
                [ctrlKey](double t) { ctrlKey->setJumpTime(Seconds{t}); });

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

void KeyframeEditorWidget::onKeyframeTimeChanged(Keyframe *key, Seconds) {
    timeSpinner_->setValue(key->getTime().count());
    sequenceEditorWidget_->setReorderNeeded();
}

void KeyframeEditorWidget::onKeyframeSelectionChanged(Keyframe *) {
    sequenceEditorWidget_->updateVisibility();
}

}  // namespace animation

}  // namespace inviwo
