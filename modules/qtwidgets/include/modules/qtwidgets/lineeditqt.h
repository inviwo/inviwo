/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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

#include <QLineEdit>  // for QLineEdit
#include <QObject>    // for Q_OBJECT, signals
#include <QSize>      // for QSize

class QPaintEvent;
class QWidget;
class QKeyEvent;

namespace inviwo {

/**
 * \class LineEditQt
 * \brief customized line edit class based on QLineEdit. Pressing Escape will emit a cancel signal.
 *  This signal can be used to revert the changes and loose focus without changing the property.
 */
class IVW_MODULE_QTWIDGETS_API LineEditQt : public QLineEdit {
    Q_OBJECT
public:
    LineEditQt(QWidget* parent = nullptr);
    virtual ~LineEditQt() = default;

    virtual QSize sizeHint() const override { return QLineEdit::minimumSizeHint(); }

    virtual void paintEvent(QPaintEvent* e) override;

signals:
    void editingCanceled();

protected:
    virtual void keyPressEvent(QKeyEvent* e) override;
};

}  // namespace inviwo
