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

#include <modules/qtwidgets/properties/propertysettingswidgetqt.h>

#include <inviwo/core/util/formats.h>  // for DataFloat64

#include <QChar>             // for QChar
#include <QDoubleValidator>  // for QDoubleValidator
#include <QWidget>           // for QWidget

namespace inviwo {

SinglePropertySetting::SinglePropertySetting(QWidget* widget, std::string label)
    : label_(new QLabel(QString::fromStdString(label), widget)), widget_(widget) {}

QLineEdit* SinglePropertySetting::addField() {
    QLineEdit* ext = new QLineEdit(widget_);
    ext->setValidator(new QDoubleValidator(widget_));
    additionalFields_.push_back(ext);
    return ext;
}

double SinglePropertySetting::getFieldAsDouble(size_t i) {
    if (i < additionalFields_.size()) {
        QLocale locale = additionalFields_[i]->locale();
        return locale.toDouble(
            additionalFields_[i]->text().remove(QChar(' ')).remove(locale.groupSeparator()));
    }
    return DataFloat64::minToDouble();
}

}  // namespace inviwo
