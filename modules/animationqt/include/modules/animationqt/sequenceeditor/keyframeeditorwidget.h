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

#ifndef IVW_KEYFRAMEEDITORWIDGET_H
#define IVW_KEYFRAMEEDITORWIDGET_H

#include <modules/animationqt/animationqtmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/properties/property.h>
#include <modules/animation/datastructures/keyframe.h>
#include <modules/animation/datastructures/keyframeobserver.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QWidget>
#include <warn/pop>

class QHBoxLayout;
class QComboBox;
class QDoubleSpinBox;

namespace inviwo {

class Property;
class PropertyWidgetQt;

namespace animation {

class SequenceEditorWidget;

class IVW_MODULE_ANIMATIONQT_API KeyframeEditorWidget : public QWidget, public KeyframeObserver {
public:
    KeyframeEditorWidget(Keyframe &keyframe, SequenceEditorWidget *parent);
    virtual ~KeyframeEditorWidget();

    virtual void onKeyframeTimeChanged(Keyframe *key, Seconds oldTime) override;

    Keyframe &getKeyframe() { return keyframe_; }

    virtual void onKeyframeSelectionChanged(Keyframe *seq) override;

private:
    Keyframe &keyframe_;
    SequenceEditorWidget *sequenceEditorWidget_{nullptr};

    std::unique_ptr<Property> property_{nullptr};
    PropertyWidgetQt *propertyWidget_{nullptr};
    QComboBox *actionWidget_{nullptr};
    QDoubleSpinBox *jumpToWidget_{nullptr};
    QHBoxLayout *layout_{nullptr};
    QDoubleSpinBox *timeSpinner_{nullptr};
};

}  // namespace animation
}  // namespace inviwo

#endif  // IVW_KEYFRAMEEDITORWIDGET_H
