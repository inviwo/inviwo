/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#include <inviwo/qt/widgets/customdoublespinboxqt.h>
#include <limits>
#include <QTimerEvent>

namespace inviwo {



CustomDoubleSpinBoxQt::CustomDoubleSpinBoxQt(QWidget* parent /*= 0*/) 
    : QDoubleSpinBox(parent)
{
    // Enables setting number of decimals to display
    displayDecimals_ = decimals();
    // Save default sizeHint before changing decimal property
    cachedSizeHint_ = QDoubleSpinBox::sizeHint();
    // Set decimals to large value so that QDoubleSpinBox does not truncate the value
    QDoubleSpinBox::setDecimals(std::numeric_limits<double>::max_exponent);
}

QString CustomDoubleSpinBoxQt::textFromValue(double value) const {
    const static QLocale local;
    return local.toString(value, 'f', displayDecimals_);
}

void CustomDoubleSpinBoxQt::setDecimals(int decimals) {
    if (decimals == displayDecimals_)
        return;

    displayDecimals_ = decimals;
    // Block so that no signals are sent
    // since the value does not change
    blockSignals(true);
    double val = value();
    QDoubleSpinBox::setDecimals(decimals);
    cachedSizeHint_ = QDoubleSpinBox::sizeHint();
    QDoubleSpinBox::setDecimals(std::numeric_limits<double>::max_exponent);
    setValue(val);
    blockSignals(false);
}


void CustomDoubleSpinBoxQt::timerEvent(QTimerEvent *event) {
    event->accept();
}

} // namespace inviwo
