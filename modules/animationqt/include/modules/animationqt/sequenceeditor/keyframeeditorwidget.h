/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2023 Inviwo Foundation
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
#pragma once

#include <modules/animationqt/animationqtmoduledefine.h>  // for IVW_MODULE_ANIMATIONQT_API

#include <inviwo/core/properties/property.h>                    // for Property
#include <modules/animation/datastructures/animationtime.h>     // for Seconds
#include <modules/animation/datastructures/keyframeobserver.h>  // for KeyframeObserver

#include <memory>  // for unique_ptr

#include <QWidget>  // for QWidget

class QComboBox;
class QDoubleSpinBox;
class QHBoxLayout;

namespace inviwo {

class PropertyWidgetQt;

namespace animation {

class Keyframe;
class SequenceEditorWidget;

class IVW_MODULE_ANIMATIONQT_API KeyframeEditorWidget : public QWidget, public KeyframeObserver {
public:
    KeyframeEditorWidget(Keyframe& keyframe, SequenceEditorWidget* parent);
    virtual ~KeyframeEditorWidget();

    virtual void onKeyframeTimeChanged(Keyframe* key, Seconds oldTime) override;

    Keyframe& getKeyframe() { return keyframe_; }

    virtual void onKeyframeSelectionChanged(Keyframe* seq) override;

private:
    Keyframe& keyframe_;
    SequenceEditorWidget* sequenceEditorWidget_{nullptr};

    std::unique_ptr<Property> property_{nullptr};
    PropertyWidgetQt* propertyWidget_{nullptr};
    QComboBox* actionWidget_{nullptr};
    QDoubleSpinBox* jumpToWidget_{nullptr};
    QHBoxLayout* layout_{nullptr};
    QDoubleSpinBox* timeSpinner_{nullptr};
};

}  // namespace animation

}  // namespace inviwo
