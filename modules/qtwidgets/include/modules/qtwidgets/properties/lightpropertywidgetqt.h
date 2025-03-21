/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>  // for IVW_MODULE_QTWIDGETS_API

#include <inviwo/core/properties/ordinalproperty.h>         // for FloatVec3Property
#include <modules/qtwidgets/properties/propertywidgetqt.h>  // for PropertyWidgetQt

#include <array>   // for array
#include <memory>  // for unique_ptr

class QCheckBox;
class QMenu;

namespace inviwo {

class EditableLabelQt;
class LightPositionWidgetQt;
template <typename Prop>
class OrdinalLikePropertySettingsWidgetQt;
template <typename T>
class OrdinalSpinBoxWidget;

class IVW_MODULE_QTWIDGETS_API LightPropertyWidgetQt : public PropertyWidgetQt {
public:
    LightPropertyWidgetQt(FloatVec3Property* property);
    virtual ~LightPropertyWidgetQt();

    virtual void updateFromProperty() override;
    virtual std::unique_ptr<QMenu> getContextMenu() override;

private:
    FloatVec3Property* property_;
    LightPositionWidgetQt* halfSphere_;
    OrdinalSpinBoxWidget<float>* radius_;
    QCheckBox* inFront_;
    std::array<OrdinalSpinBoxWidget<float>*, 3> cartesian_;

    EditableLabelQt* label_;
    OrdinalLikePropertySettingsWidgetQt<FloatVec3Property>* settings_;
};

}  // namespace inviwo
