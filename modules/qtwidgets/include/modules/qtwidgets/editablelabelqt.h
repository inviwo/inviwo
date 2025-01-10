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

#pragma once

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>  // for IVW_MODULE_QTWIDGETS_API

#include <inviwo/core/properties/propertyobserver.h>  // for PropertyObserver

#include <string>  // for string

#include <QObject>  // for Q_OBJECT, signals
#include <QSize>    // for QSize
#include <QString>  // for QString
#include <QWidget>  // for QWidget

class QEvent;
class QLabel;
class QLineEdit;
class QMouseEvent;
class QResizeEvent;

namespace inviwo {

class Property;
class PropertyWidgetQt;

class IVW_MODULE_QTWIDGETS_API EditableLabelQt : public QWidget, public PropertyObserver {
    Q_OBJECT
public:
    EditableLabelQt(PropertyWidgetQt* parent, Property* property, bool shortenText = true);
    EditableLabelQt(PropertyWidgetQt* parent, const std::string& text, bool shortenText = true);
    std::string getText();
    void setText(const std::string& txt);
    void setShortenText(bool shorten);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void edit();

protected:
    virtual bool event(QEvent* event) override;
    virtual void resizeEvent(QResizeEvent*) override;

private:
    void updateLabel(const std::string& text);
    void mouseDoubleClickEvent(QMouseEvent* e) override;
    virtual void onSetDisplayName(Property* property, const std::string& displayName) override;
    QString shortenText(const std::string& text);

    QLineEdit* getLineEdit();

    QLabel* label_;
    QLineEdit* lineEdit_;
    std::string text_;
    Property* property_;
    PropertyWidgetQt* propertyWidget_;
    bool shortenText_;

signals:
    void textChanged();
};

}  // namespace inviwo
