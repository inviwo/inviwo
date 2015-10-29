/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/qt/widgets/customdoublespinboxqt.h>
#include <inviwo/qt/widgets/editablelabelqt.h>
#include <inviwo/qt/widgets/lightpositionwidgetqt.h>
#include <inviwo/qt/widgets/properties/propertywidgetqt.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QtCore/qmath.h>
#include <QSpinBox>
#include <warn/pop>

namespace inviwo {

class IVW_QTWIDGETS_API LightPropertyWidgetQt : public PropertyWidgetQt {

    #include <warn/push>
    #include <warn/ignore/all>
    Q_OBJECT
    #include <warn/pop>

public:
    LightPropertyWidgetQt(FloatVec3Property* property);
    virtual ~LightPropertyWidgetQt();

    void updateFromProperty();

private:
    FloatVec3Property* property_;
    LightPositionWidgetQt* lightWidget_;
    CustomDoubleSpinBoxQt* radiusSpinBox_;
    EditableLabelQt* label_;

    void generateWidget();

public slots:
    void onPositionLightWidgetChanged();
    void onRadiusSpinBoxChanged(double radius);
};

} // namespace

#endif // IVW_LightPropertyWidgetQt_H
