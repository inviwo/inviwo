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

#include <modules/qtwidgets/tf/tfcoloredit.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <inviwo/core/util/colorconversion.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QSizePolicy>
#include <warn/pop>

namespace inviwo {

TFColorEdit::TFColorEdit(QWidget* parent) : ColorLineEdit(parent) {
    setRepresentation(ColorRepresentation::Hexadecimal);

    connect(this, &ColorLineEdit::colorChanged, this, [this]() {
        // QColor(QString) should only be used for 6-digit hex codes, since
        // 8-digit hex codes in Qt are in the form of #AARRGGBB while Inviwo uses #RRGGBBAA
        emit colorChanged(utilqt::toQColor(getColor<vec3>()));
    });

    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred));
}

QSize TFColorEdit::sizeHint() const { return QSize(18, 18); }

void TFColorEdit::setColor(const QColor& color, bool ambiguous) {
    if (ambiguous) {
        setInvalid(true);
    } else {
        ColorLineEdit::setColor(utilqt::tovec3(color), ColorRepresentation::Hexadecimal);
    }
}

}  // namespace inviwo
