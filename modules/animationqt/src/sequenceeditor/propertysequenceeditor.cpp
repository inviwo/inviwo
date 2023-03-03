/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2023 Inviwo Foundation
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

#include <inviwo/core/common/factoryutil.h>                            // for getPropertyWidgetF...
#include <inviwo/core/properties/cameraproperty.h>                     // for CameraProperty
#include <inviwo/core/properties/property.h>                           // for Property
#include <inviwo/core/properties/propertywidgetfactory.h>              // for PropertyWidgetFactory
#include <inviwo/core/util/stringconversion.h>                         // for toString
#include <modules/animation/animationmanager.h>                        // for AnimationManager
#include <modules/animation/datastructures/animationtime.h>            // for Seconds
#include <modules/animation/datastructures/easing.h>                   // for operator++, operat...
#include <modules/animation/datastructures/keyframe.h>                 // for Keyframe
#include <modules/animation/datastructures/keyframeobserver.h>         // for KeyframeObserver
#include <modules/animation/datastructures/keyframesequence.h>         // for KeyframeSequence
#include <modules/animation/datastructures/propertytrack.h>            // for BasePropertyTrack
#include <modules/animation/datastructures/track.h>                    // for Track
#include <modules/animation/datastructures/valuekeyframesequence.h>    // for ValueKeyframeSequence
#include <modules/animation/factories/interpolationfactory.h>          // for InterpolationFactory
#include <modules/animation/factories/interpolationfactoryobject.h>    // for InterpolationFacto...
#include <modules/animation/interpolation/interpolation.h>             // for Interpolation
#include <modules/animationqt/sequenceeditor/sequenceeditorwidget.h>   // for SequenceEditorWidget
#include <modules/qtwidgets/editablelabelqt.h>                         // for EditableLabelQt
#include <modules/qtwidgets/inviwoqtutils.h>                           // for toQString, refSpacePx
#include <modules/qtwidgets/properties/collapsiblegroupboxwidgetqt.h>  // for CollapsibleGroupBo...
#include <modules/qtwidgets/properties/propertywidgetqt.h>             // for PropertyWidgetQt

#include <algorithm>         // for none_of
#include <cstddef>           // for size_t
#include <functional>        // for __base
#include <initializer_list>  // for begin, end
#include <memory>            // for unique_ptr, unique...
#include <vector>            // for vector

#include <QComboBox>       // for QComboBox
#include <QDoubleSpinBox>  // for QDoubleSpinBox
#include <QFont>           // for QFont
#include <QGridLayout>     // for QGridLayout
#include <QHBoxLayout>     // for QHBoxLayout
#include <QLabel>          // for QLabel
#include <QSignalBlocker>  // for QSignalBlocker
#include <QVBoxLayout>     // for QVBoxLayout
#include <QVariant>        // for QVariant
#include <QWidget>         // for QWidget

class QDoubleSpinBox;

namespace inviwo {

namespace animation {

namespace {

class PropertyEditorWidget : public QWidget, public KeyframeObserver {
public:
    PropertyEditorWidget(Keyframe& keyframe, SequenceEditorWidget* parent)
        : QWidget(parent), keyframe_(keyframe), sequenceEditorWidget_(parent) {

        auto& propTrack = dynamic_cast<BasePropertyTrack&>(parent->getTrack());

        setObjectName("KeyframeEditorWidget");
        setVisible(keyframe_.isSelected());
        keyframe.addObserver(this);

        auto layout = new QHBoxLayout();
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(utilqt::refSpacePx(this));

        timeSpinner_ = new QDoubleSpinBox();
        timeSpinner_->setValue(keyframe.getTime().count());
        timeSpinner_->setSuffix("s");
        timeSpinner_->setSingleStep(0.1);
        timeSpinner_->setDecimals(3);

        connect(timeSpinner_,
                static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this,
                [this](double t) { keyframe_.setTime(Seconds(t)); });

        layout->addWidget(timeSpinner_);

        property_.reset(propTrack.getProperty()->clone());
        propTrack.setPropertyFromKeyframe(property_.get(), &keyframe);
        property_->onChange([p = property_.get(), &propTrack, k = &keyframe_]() {
            propTrack.setKeyframeFromProperty(p, k);
        });
        property_->setOwner(nullptr);

        auto propWidget = util::getPropertyWidgetFactory()->create(property_.get());
        propertyWidget_ = static_cast<PropertyWidgetQt*>(propWidget.release());

        if (auto label = propertyWidget_->findChild<EditableLabelQt*>()) {
            // Do not repeat information (Track name in PropertySequenceEditor) for each property
            label->setVisible(false);
        }
        if (auto collapsibleWidget = dynamic_cast<CollapsibleGroupBoxWidgetQt*>(propertyWidget_)) {
            collapsibleWidget->initState();
            if (auto cameraProperty = dynamic_cast<CameraProperty*>(property_.get())) {
                // HACK: Only show relevant properties for the CameraTrack
                // We should delegate this in case more properties need this
                std::vector<Property*> properties = cameraProperty->getPropertiesRecursive();
                for (auto prop : properties) {
                    const auto keepProperties = {"lookFrom", "lookTo", "lookUp"};
                    if (std::none_of(
                            std::begin(keepProperties), std::end(keepProperties),
                            [prop](const auto& v) { return v == prop->getIdentifier(); })) {
                        prop->setVisible(false);
                    }
                }
            }
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

    std::unique_ptr<Property> property_{nullptr};
    PropertyWidgetQt* propertyWidget_{nullptr};
    QDoubleSpinBox* timeSpinner_{nullptr};
};

}  // namespace

PropertySequenceEditor::PropertySequenceEditor(KeyframeSequence& sequence, Track& track,
                                               AnimationManager& manager)
    : SequenceEditorWidget(sequence, track, manager) {

    sequence.addObserver(this);

    const auto space = utilqt::refSpacePx(this);
    setContentsMargins(space, space, 0, space);

    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(space);

    setLayout(layout);

    auto label = new QLabel(utilqt::toQString(track_.getName()));
    auto font = label->font();
    font.setBold(true);
    label->setFont(font);
    layout->addWidget(label);

    keyframesLayout_ = new QVBoxLayout();
    keyframesLayout_->setContentsMargins(0, 0, 0, 0);
    keyframesLayout_->setSpacing(space);

    layout->addLayout(keyframesLayout_);

    auto sublayout = new QGridLayout();
    layout->addLayout(sublayout);
    sublayout->setContentsMargins(0, 0, 0, 0);
    sublayout->setSpacing(7);
    interpolation_ = new QComboBox();
    sublayout->addWidget(new QLabel("Interpolation"), 0, 0);
    sublayout->addWidget(interpolation_, 0, 1);

    auto& valseq = dynamic_cast<ValueKeyframeSequence&>(sequence);
    for (auto interpObj : valseq.getSupportedInterpolations(manager.getInterpolationFactory())) {
        auto ip = interpObj->create();
        interpolation_->addItem(utilqt::toQString(ip->getName()),
                                QVariant(utilqt::toQString(ip->getClassIdentifier())));
        if (valseq.getInterpolation().getClassIdentifier() == ip->getClassIdentifier()) {
            interpolation_->setCurrentIndex(interpolation_->count() - 1);
        }
    }

    connect(interpolation_, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this, &valseq, &manager](int) {
                const auto id = utilqt::fromQString(interpolation_->currentData().toString());
                valseq.setInterpolation(manager.getInterpolationFactory().create(id));
            });

    easingComboBox_ = new QComboBox();
    sublayout->addWidget(new QLabel("Easing"), 1, 0);
    sublayout->addWidget(easingComboBox_, 1, 1);

    for (auto e = easing::FirstEasingType; e <= easing::LastEasingType; ++e) {
        easingComboBox_->addItem(utilqt::toQString(toString(e)), QVariant(static_cast<int>(e)));
        if (valseq.getEasingType() == e) {
            easingComboBox_->setCurrentIndex(easingComboBox_->count() - 1);
        }
    }

    connect(easingComboBox_, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this,
            [&valseq](int index) { valseq.setEasingType(static_cast<easing::EasingType>(index)); });

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

void PropertySequenceEditor::onValueKeyframeSequenceInterpolationChanged(
    ValueKeyframeSequence* seq) {

    auto id = utilqt::toQString(seq->getInterpolation().getClassIdentifier());
    auto ind = interpolation_->findData(id);
    interpolation_->setCurrentIndex(ind);
}

std::string PropertySequenceEditor::classIdentifier() {
    return "org.inviwo.animation.PropertySequenceEditor";
}

}  // namespace animation

}  // namespace inviwo
