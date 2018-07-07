/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018 Inviwo Foundation
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

#include <modules/animationqt/sequenceeditor/propertysequenceeditor.h>

#include <modules/animationqt/sequenceeditor/keyframeeditorwidget.h>
#include <modules/animation/datastructures/valuekeyframe.h>
#include <modules/animation/datastructures/valuekeyframesequence.h>
#include <modules/animation/datastructures/propertytrack.h>

#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/util/stringconversion.h>

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
#include <warn/pop>

namespace inviwo {

namespace animation {

namespace {

class PropertyEditorWidget : public QWidget, public KeyframeObserver {
public:
    PropertyEditorWidget(Keyframe &keyframe, SequenceEditorWidget *parent)
        : QWidget(parent), keyframe_(keyframe), sequenceEditorWidget_(parent) {

        auto propTrack = dynamic_cast<BasePropertyTrack *>(&parent->getTrack());
        if (!propTrack) throw Exception("error");

        setObjectName("KeyframeEditorWidget");

        keyframe.addObserver(this);

        auto layout = new QHBoxLayout();

        timeSpinner_ = new QDoubleSpinBox();
        timeSpinner_->setValue(keyframe.getTime().count());
        timeSpinner_->setSuffix("s");
        timeSpinner_->setSingleStep(0.1);
        timeSpinner_->setDecimals(3);

        connect(timeSpinner_,
                static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this,
                [this](double t) { keyframe_.setTime(Seconds(t)); });

        layout->addWidget(timeSpinner_);

        auto baseProperty = propTrack->getProperty();
        property_.reset(baseProperty->clone());
        propTrack->setOtherProperty(property_.get(), &keyframe);
        property_->onChange([b = baseProperty, p = property_.get(), t = propTrack,
                             k = &keyframe_]() { t->updateKeyframeFromProperty(p, k); });
        property_->setOwner(nullptr);

        auto propWidget =
            util::getInviwoApplication()->getPropertyWidgetFactory()->create(property_.get());
        propertyWidget_ = static_cast<PropertyWidgetQt *>(propWidget.release());

        if (auto label = propertyWidget_->findChild<EditableLabelQt *>()) {
            label->setVisible(false);
        }
        layout->addWidget(propertyWidget_);

        setLayout(layout);
    }

    virtual ~PropertyEditorWidget() {
        if (propertyWidget_) {
            // We need to manually remove and delete the propertyWidget_ since the destructor of
            // propertyWidget_ tries to use the property. If we do not remove it, it will be
            // destroyed in QWidgets destructor which is called after this destructor, hence, the
            // property has been destroyed.
            delete propertyWidget_;
        }
    }

    virtual void onKeyframeTimeChanged(Keyframe *key, Seconds) override {
        timeSpinner_->setValue(key->getTime().count());
        sequenceEditorWidget_->setReorderNeeded();
    }

    Keyframe &getKeyframe() { return keyframe_; }

    virtual void onKeyframeSelectionChanged(Keyframe *) override {
        sequenceEditorWidget_->updateVisibility();
    }

private:
    Keyframe &keyframe_;
    SequenceEditorWidget *sequenceEditorWidget_{nullptr};

    std::unique_ptr<Property> property_{nullptr};
    PropertyWidgetQt *propertyWidget_{nullptr};
    QDoubleSpinBox *timeSpinner_{nullptr};
};

}  // namespace

PropertySequenceEditor::PropertySequenceEditor(KeyframeSequence &sequence, Track &track)
    : SequenceEditorWidget(sequence, track) {

    sequence_.addObserver(this);

    auto layout = new QVBoxLayout();
    setLayout(layout);

    auto label = new QLabel(utilqt::toQString(track_.getName()));
    auto font = label->font();
    font.setBold(true);
    label->setFont(font);
    layout->addWidget(label);

    keyframesLayout_ = new QVBoxLayout();
    layout->addLayout(keyframesLayout_);

    if (auto valseq = dynamic_cast<ValueKeyframeSequence *>(&sequence_)) {
        auto easingLayout = new QHBoxLayout();
        layout->addLayout(easingLayout);

        easingComboBox_ = new QComboBox();
        easingLayout->addWidget(new QLabel("Easing: "));
        easingLayout->addWidget(easingComboBox_);

        for (auto e = easing::FirstEasingType; e <= easing::LastEasingType; ++e) {
            easingComboBox_->addItem(utilqt::toQString(toString(e)), QVariant(static_cast<int>(e)));
            if (valseq->getEasingType() == e) {
                easingComboBox_->setCurrentIndex(easingComboBox_->count() - 1);
            }
        }

        void (QComboBox::*signal)(int) = &QComboBox::currentIndexChanged;
        connect(easingComboBox_, signal, [valseq](int index) {
            valseq->setEasingType(static_cast<easing::EasingType>(index));
        });
    }

    for (size_t i = 0; i < sequence_.size(); i++) {
        onKeyframeAdded(&sequence_[i], &sequence_);
    }

    updateVisibility();
}

QWidget* PropertySequenceEditor::create(Keyframe* key) {
    return new PropertyEditorWidget(*key, this);
}

void PropertySequenceEditor::onValueKeyframeSequenceEasingChanged(ValueKeyframeSequence* seq) {
    QSignalBlocker block(easingComboBox_);
    auto index = easingComboBox_->findData(QVariant(static_cast<int>(seq->getEasingType())));
    easingComboBox_->setCurrentIndex(index);
}

std::string PropertySequenceEditor::classIdentifier() {
    return "org.inviwo.animation.PropertySequenceEditor";
}

}  // namespace animation

}  // namespace inviwo
