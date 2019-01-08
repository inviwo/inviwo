/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

#ifndef IVW_LIGHTPROPERTYWIDGETQT_H
#define IVW_LIGHTPROPERTYWIDGETQT_H

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <modules/qtwidgets/customdoublespinboxqt.h>
#include <modules/qtwidgets/editablelabelqt.h>
#include <modules/qtwidgets/lightpositionwidgetqt.h>
#include <modules/qtwidgets/properties/propertywidgetqt.h>

class CustomDoubleSpinBoxQt;

namespace inviwo {

class IVW_MODULE_QTWIDGETS_API LightPropertyWidgetQt : public PropertyWidgetQt {
public:
    LightPropertyWidgetQt(FloatVec3Property* property);
    virtual ~LightPropertyWidgetQt();

    virtual void updateFromProperty() override;

private:
    FloatVec3Property* property_;
    LightPositionWidgetQt* lightWidget_;
    CustomDoubleSpinBoxQt* radiusSpinBox_;
    EditableLabelQt* label_;

    void onPositionLightWidgetChanged();
    void onRadiusSpinBoxChanged(double radius);
};

}  // namespace inviwo

#endif  // IVW_LightPropertyWidgetQt_H
