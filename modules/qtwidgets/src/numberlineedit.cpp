/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <modules/qtwidgets/numberlineedit.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QFont>
#include <QFontMetrics>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QEvent>
#include <QLocale>
#include <QStyleOptionSpinBox>
#include <QStyle>
#include <QApplication>
#include <warn/pop>

#include <numeric>
#include <unordered_map>
#include <array>

namespace inviwo {

class NumberLineEditPrivate {
public:
    NumberLineEditPrivate();

    void clear();

    int getPrecision(int availableWidth, uint fontHash, const QFontMetrics &fm);

    QString formatAsScientific(double value, int availableWidth, uint fontHash,
                               const QFontMetrics &fm);
    QString formatAsScientific(double value, int precision);
    QString formatAsNonscientific(double v) const;
    QString formatAsInt(double value) const;

    void updateLocale();

    QLocale getLocale() const;

private:
    QLocale locale_;
    // hash map with qHash(QFont) as a key and the value mapping the available width of a QLineEdit
    // to the number of digits fitting inside the widget
    std::unordered_map<unsigned int, std::array<int, 513>> widthToDigits_;
};

NumberLineEditPrivate::NumberLineEditPrivate() { updateLocale(); }

void NumberLineEditPrivate::clear() { widthToDigits_.clear(); }

int NumberLineEditPrivate::getPrecision(int availableWidth, uint fontHash, const QFontMetrics &fm) {
    availableWidth = std::min(std::max(availableWidth, 0), 512);
    auto it = widthToDigits_.find(fontHash);
    if (it == widthToDigits_.end()) {
        it = widthToDigits_
                 .insert({fontHash,
                          []() {
                              std::array<int, 513> ret;
                              ret.fill(-1);
                              return ret;
                          }()})
                 .first;
    } else {
        const int digits = it->second[availableWidth];
        if (digits > -1) {
            return digits;
        }
    }
    const double refValue = -8.88888888888888888888;

    int precision = 16;
    QString str = locale_.toString(refValue, 'g', precision);
    while ((precision > 0) && (fm.boundingRect(str).width() > availableWidth)) {
        str = locale_.toString(refValue, 'g', --precision);
    }
    it->second[availableWidth] = precision;
    return precision;
}

QString NumberLineEditPrivate::formatAsScientific(double value, int availableWidth, uint fontHash,
                                                  const QFontMetrics &fm) {
    return formatAsScientific(value, getPrecision(availableWidth, fontHash, fm));
}

QString NumberLineEditPrivate::formatAsScientific(double value, int precision) {
    return locale_.toString(value, 'g', precision);
}

QString NumberLineEditPrivate::formatAsNonscientific(double value) const {
    // default representation has 8 decimals after the decimal point, e.g. a value of
    // 0.123456789 yields "0.12345679". Each order of magnitude reduces the number of decimals by
    // one, i.e. 100.12345678 will be represented as "100.123457". Numbers smaller than 1e-6 are
    // shown in scientific representation.
    const int visibleDigits = 8;
    const double scientificRepThreshold = 1e-6;

    if (std::abs(value) < scientificRepThreshold) {
        return locale_.toString(value, 'g', visibleDigits + 1);
    } else {
        const int int_digits =
            static_cast<int>(std::floor(std::log10(std::max(std::abs(value), 1.0)))) + 1;
        QString str = locale_.toString(value, 'f', std::max(visibleDigits + 1 - int_digits, 1));
        if (str.contains(locale_.decimalPoint())) {
            // remove trailing zeros
            while (*str.rbegin() == '0') {
                str.chop(1);
            }
            if (*str.rbegin() == locale_.decimalPoint()) {
                str.chop(1);
            }
        }
        return str;
    }
}

QString NumberLineEditPrivate::formatAsInt(double value) const {
    return locale_.toString(static_cast<long long int>(value));
}

void NumberLineEditPrivate::updateLocale() {
    locale_ = QLocale::system();
    locale_.setNumberOptions(locale_.numberOptions().setFlag(QLocale::OmitGroupSeparator, true));
}

QLocale NumberLineEditPrivate::getLocale() const { return locale_; }

std::unique_ptr<NumberLineEditPrivate> NumberLineEdit::nlePrivate_(new NumberLineEditPrivate);

NumberLineEdit::NumberLineEdit(QWidget *parent) : NumberLineEdit(false, parent) {}

NumberLineEdit::NumberLineEdit(bool intMode, QWidget *parent)
    : QDoubleSpinBox(parent), integerMode_(intMode) {
    validator_ = new QDoubleValidator(this);
    validator_->setNotation(QDoubleValidator::ScientificNotation);
    lineEdit()->setValidator(validator_);
    // need a high precision in QDoubleSpinBox since min and max values are rounded using the
    // number of decimals
    QDoubleSpinBox::setDecimals(20);
    setFocusPolicy(Qt::StrongFocus);
}

NumberLineEdit::~NumberLineEdit() = default;

QSize NumberLineEdit::sizeHint() const {
    QSize hint = QDoubleSpinBox::sizeHint();
    hint.setWidth(hint.height());
    return hint;
}

QSize NumberLineEdit::minimumSizeHint() const {
    if (cachedMinimumSizeHint_.isEmpty()) {
        ensurePolished();
        QSize hint(minimumWidth_, lineEdit()->minimumSizeHint().height());
        QStyleOptionSpinBox opt;
        initStyleOption(&opt);

        QSize sizeContents = style()->sizeFromContents(QStyle::CT_SpinBox, &opt, hint, this);
        // For some odd reason, sizeFromContents always includes the spinbox buttons
        if (opt.buttonSymbols == QAbstractSpinBox::NoButtons) {
            sizeContents.setWidth(sizeContents.width() -
                                  static_cast<int>(sizeContents.height() / 1.2));
        }
        cachedMinimumSizeHint_ = sizeContents.expandedTo(QApplication::globalStrut());
    }
    return cachedMinimumSizeHint_;
}

bool NumberLineEdit::isValid() const { return !invalid_; }

void NumberLineEdit::setInvalid(bool invalid) {
    invalid_ = invalid;
    setValue(value());
}

QString NumberLineEdit::textFromValue(double value) const {
    if (invalid_) {
        return "-";
    }

    auto formatNumber = [&](double v) {
        if (integerMode_) {
            return nlePrivate_->formatAsInt(v);
        } else {
            return nlePrivate_->formatAsNonscientific(v);
        }
    };

    if (abbreviated_) {
        ensurePolished();
        const int availableWidth = lineEdit()->geometry().width() - 4;
        QFontMetrics fm = lineEdit()->fontMetrics();

        auto str = formatNumber(value);
        if (fm.boundingRect(str).width() < availableWidth) {
            return str;
        }

        str = nlePrivate_->formatAsScientific(value, availableWidth, qHash(lineEdit()->font()), fm);
        if (fm.boundingRect(str).width() > availableWidth) {
            str = nlePrivate_->formatAsScientific(value, 1);
        }
        return str;
    }
    return formatNumber(value);
}

double NumberLineEdit::valueFromText(const QString &str) const {
    bool ok = false;
    double value = nlePrivate_->getLocale().toDouble(str, &ok);
    return ok ? value : QDoubleSpinBox::value();
}

QValidator::State NumberLineEdit::validate(QString &text, int &pos) const {
    return validator_->validate(text, pos);
}

void NumberLineEdit::setDecimals(int decimals) { visibleDecimals_ = decimals; }

void NumberLineEdit::setMinimum(double min) { QDoubleSpinBox::setMinimum(min); }

void NumberLineEdit::setMaximum(double max) { QDoubleSpinBox::setMaximum(max); }

void NumberLineEdit::setRange(double min, double max) { QDoubleSpinBox::setRange(min, max); }

void NumberLineEdit::timerEvent(QTimerEvent *event) { event->accept(); }

void NumberLineEdit::focusInEvent(QFocusEvent *e) {
    abbreviated_ = false;
    lineEdit()->setText(textFromValue(value()));
    QDoubleSpinBox::focusInEvent(e);
}

void NumberLineEdit::focusOutEvent(QFocusEvent *e) {
    abbreviated_ = true;
    QDoubleSpinBox::focusOutEvent(e);
}

void NumberLineEdit::resizeEvent(QResizeEvent *e) {
    QDoubleSpinBox::resizeEvent(e);
    setSpecialValueText(specialValueText());
}

void NumberLineEdit::changeEvent(QEvent *e) {
    if (e->type() == QEvent::LocaleChange) {
        nlePrivate_->updateLocale();
    } else if (e->type() == QEvent::StyleChange) {
        cachedMinimumSizeHint_ = QSize();
        updateGeometry();
    }
    QDoubleSpinBox::changeEvent(e);
}

void NumberLineEdit::wheelEvent(QWheelEvent *e) {
    if (hasFocus() && !invalid_) QDoubleSpinBox::wheelEvent(e);
}

}  // namespace inviwo
