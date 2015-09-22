/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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

#ifndef IVW_DOUBLE_SPIN_BOX_QT_H
#define IVW_DOUBLE_SPIN_BOX_QT_H

#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QDoubleSpinBox>
#include <warn/pop>

class QTimerEvent;

namespace inviwo {

/** \class CustomDoubleSpinBoxQt
 *
 * Enables displaying a value with a certain precision without truncation the actual value, which QDoubleSpinBox does.
 */
class IVW_QTWIDGETS_API CustomDoubleSpinBoxQt : public QDoubleSpinBox {
    Q_OBJECT
public:
    explicit CustomDoubleSpinBoxQt(QWidget* parent = 0);
    virtual QString textFromValue(double value) const;

    /**
     * Override QDoubleSpinBox size hint so that
     * it does not use the one for many decimals.
     *
     */
    QSize sizeHint() const { return cachedSizeHint_; }
    /**
     * Override QDoubleSpinBox decimals so that
     * we can cache size and decimals to display.
     *
     */
    virtual void setDecimals(int decimals);

    /** 
     * \brief Overrides the timerEvent to prevent
     * spinbox to be updated twice in case of 
     * calculations being slows
     */
    virtual void timerEvent(QTimerEvent *event);
protected:
    QSize cachedSizeHint_;
    int displayDecimals_;

};

}//namespace

#endif // IVW_DOUBLE_SPIN_BOX_QT_H