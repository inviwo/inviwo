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

#include <modules/qtwidgets/properties/colorlineedit.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/core/util/colorconversion.h>
#include <inviwo/core/util/assertion.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <QLocale>
#include <QStyle>
#include <QEvent>
#include <QKeyEvent>
#include <warn/pop>

namespace inviwo {

ColorLineEdit::ColorLineEdit(QWidget *parent)
    : QLineEdit(parent), validator_(new QRegularExpressionValidator(this)) {
    setObjectName("ColorLineEdit");
    updateRegExp();
    setValidator(validator_);

    connect(this, &QLineEdit::editingFinished, this, &ColorLineEdit::updateColor);

    connect(this, &QLineEdit::textChanged, this, [&](const QString &) {
        if (hasFocus()) {
            invalid_ = false;
            setProperty("input", hasAcceptableInput() ? "valid" : "invalid");
            style()->unpolish(this);
            style()->polish(this);
        }
    });
}

void ColorLineEdit::setColor(ivec3 v, ColorRepresentation rep) {
    color_ = dvec4(glm::clamp(v, ivec3(0), ivec3(255)), 1.0) / 255.0;
    hasAlpha_ = false;
    representation_ = rep;
    invalid_ = false;
    updateText();
}

void ColorLineEdit::setColor(ivec4 v, ColorRepresentation rep) {
    color_ = dvec4(glm::clamp(v, ivec4(0), ivec4(255))) / 255.0;
    hasAlpha_ = true;
    representation_ = rep;
    invalid_ = false;
    updateText();
}

void ColorLineEdit::setColor(vec3 v, ColorRepresentation rep) { setColor(dvec3(v), rep); }

void ColorLineEdit::setColor(vec4 v, ColorRepresentation rep) { setColor(dvec4(v), rep); }

void ColorLineEdit::setColor(dvec3 v, ColorRepresentation rep) {
    color_ = dvec4(v, 1.0);
    hasAlpha_ = false;
    representation_ = rep;
    invalid_ = false;
    updateText();
}

void ColorLineEdit::setColor(dvec4 v, ColorRepresentation rep) {
    color_ = v;
    hasAlpha_ = true;
    representation_ = rep;
    invalid_ = false;
    updateText();
}

bool ColorLineEdit::hasAlpha() const { return hasAlpha_; }

void ColorLineEdit::setRepresentation(ColorRepresentation rep) {
    if (rep != representation_) {
        representation_ = rep;
        updateText();
    }
}

ColorLineEdit::ColorRepresentation ColorLineEdit::getRepresentation() const {
    return representation_;
}

bool ColorLineEdit::isValid() const { return !invalid_; }

void ColorLineEdit::setInvalid(bool invalid) {
    if (invalid_ == invalid) return;

    invalid_ = invalid;
    updateText();
}

void ColorLineEdit::changeEvent(QEvent *event) {
    switch (event->type()) {
        case QEvent::LocaleChange:
            updateRegExp();
            break;
        default:
            break;
    }
    QLineEdit::changeEvent(event);
}

void ColorLineEdit::focusInEvent(QFocusEvent *event) {
    setProperty("input", hasAcceptableInput() ? "valid" : "invalid");
    style()->unpolish(this);
    style()->polish(this);

    QLineEdit::focusInEvent(event);
}

void ColorLineEdit::focusOutEvent(QFocusEvent *event) {
    if (!hasAcceptableInput()) {
        // discard changes if invalid
        updateText();
    }

    setProperty("input", "none");
    style()->unpolish(this);
    style()->polish(this);

    QLineEdit::focusOutEvent(event);
}

void ColorLineEdit::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        // discard changes
        updateText();
        event->accept();
    }
    QLineEdit::keyPressEvent(event);
}

void ColorLineEdit::updateText() {
    if (invalid_) {
        setText("-");
        return;
    }

    // create appropriate textual representation
    QString str;
    switch (representation_) {
        case inviwo::ColorLineEdit::ColorRepresentation::Integer: {
            auto c = util::glm_convert<ivec4>(color_ * 255.0);
            str = QString("%1 %2 %3").arg(c.r).arg(c.g).arg(c.b);
            if (hasAlpha_) {
                str.append(QString(" %1").arg(c.a));
            }
        } break;
        case inviwo::ColorLineEdit::ColorRepresentation::Hexadecimal:
            if (hasAlpha_) {
                str = utilqt::toQString(color::rgba2hex(vec4(color_)));
            } else {
                str = utilqt::toQString(color::rgb2hex(vec3(color_)));
            }
            break;
        case inviwo::ColorLineEdit::ColorRepresentation::FloatingPoint:
        default:
            str = QString("%L1 %L2 %L3")
                      .arg(color_.r, 0, 'f', 3)
                      .arg(color_.g, 0, 'f', 3)
                      .arg(color_.b, 0, 'f', 3);
            if (hasAlpha_) {
                str.append(QString(" %L1").arg(color_.a, 0, 'f', 3));
            }
            break;
    }
    setText(str);
}

void ColorLineEdit::updateColor() {
    QStringList tokens = text().split(QRegularExpression("\\s+"));

    ivwAssert((tokens.size() == 1) || (tokens.size() == 3) || (tokens.size() == 4),
              "Invalid number of color components");
    ivwAssert((tokens.size() > 1) || ((tokens.size() == 1) && tokens.front().startsWith("#")),
              "Invalid single component (expected hex color code starting with '#')");

    if (tokens.size() == 1) {
        // it is a hex color code
        color_ = color::hex2rgba(utilqt::fromQString(tokens.front()));
    } else {
        auto locale = QLocale::system();

        dvec4 color(0.0);
        for (int i = 0; i < tokens.size(); ++i) {
            color[i] = locale.toDouble(tokens[i]);
        }

        // detect type of representation
        // If it contains at least one decimal point it must be a float range color, i.e. [0,1]
        if (tokens.filter(locale.decimalPoint()).size() == 0) {
            // assuming uint8 representation, renormalize values to [0,1]
            color /= 255.0;
        }
        if (!hasAlpha_) {
            color.a = 1.0;
        }
        color_ = color;
    }

    // ensure that line edit shows proper color representation
    updateText();
    emit colorChanged();
}

void ColorLineEdit::updateRegExp() {
    QString decimalPoint = QLocale::system().decimalPoint();
    if (decimalPoint == ".") {
        decimalPoint = "\\.";
    }

    // create a regular expression matching either
    //   - hex color codes #RRGGBB, #RRGGBBAA, #RGB, #RGBA
    //   - uint8 colors (rgb, rgba)
    //   - double colors [0,1] (rgb, rgba)
    validator_->setRegularExpression(QRegularExpression(
        QString(
            "((((?:^|\\s+)#([0-9a-fA-F]{2}){3,4})|((?:^|\\s+)#[0-9a-fA-F]{3,4}))"
            "|(((?:^|\\s+)([0-9]|([1-9][0-9])|(1[0-9][0-9])|(2[0-4][0-9])|(25[0-5]))){3,4})"
            "|((?:^|\\s+)([-+]?(((\\d+%1\\d*)|(%1\\d+))+([eE][-+]?\\d+)?|\\d+([eE][-+]?\\d+)?)))"
            "{3,4})\\s*")
            .arg(decimalPoint)));
}

template <>
dvec3 ColorLineEdit::getColor<dvec3>() const {
    return color_;
}
template <>
dvec4 ColorLineEdit::getColor<dvec4>() const {
    return color_;
}

}  // namespace inviwo
