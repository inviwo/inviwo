/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#ifndef IVW_TFCOLOREDIT_H
#define IVW_TFCOLOREDIT_H

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <modules/qtwidgets/properties/colorlineedit.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QLineEdit>
#include <QColor>
#include <warn/pop>

namespace inviwo {

/**
 * \class TFColorEdit
 * \brief widget in TF dialog for entering six digit HTML hex color codes
 */
class IVW_MODULE_QTWIDGETS_API TFColorEdit : public ColorLineEdit {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    TFColorEdit(QWidget* parent = nullptr);
    virtual ~TFColorEdit() = default;

    virtual QSize sizeHint() const override;

    /**
     * set the color value of the line edit, if the value is ambiguous a "-" will be shown.
     * Otherwise the rgb components of the color are shown as hexadecimal color code, e.g.
     * "#f9a033".
     *
     * @param color   rgb components are converted into a hexadecimal color code
     * @param ambiguous     whether the color has a name or not
     */
    void setColor(const QColor& color, bool ambiguous);

signals:
    /**
     * signal when a new valid color hex code has been entered.
     */
    void colorChanged(const QColor& color);
};

}  // namespace inviwo

#endif  // IVW_TFCOLOREDIT_H
