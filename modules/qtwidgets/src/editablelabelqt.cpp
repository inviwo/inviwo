/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2022 Inviwo Foundation
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

#include <modules/qtwidgets/editablelabelqt.h>

#include <inviwo/core/properties/property.h>                // for Property
#include <modules/qtwidgets/inviwoqtutils.h>                // for emToPx
#include <modules/qtwidgets/properties/propertywidgetqt.h>  // for PropertyWidgetQt

#include <memory>                                           // for unique_ptr

#include <warn/push>
#include <warn/ignore/all>
#include <QAction>                                          // for QAction
#include <QEvent>                                           // for QEvent
#include <QFontMetrics>                                     // for QFontMetrics
#include <QHBoxLayout>                                      // for QHBoxLayout
#include <QLabel>                                           // for QLabel
#include <QLayout>                                          // for QLayout
#include <QLineEdit>                                        // for QLineEdit
#include <QMenu>                                            // for QMenu
#include <QMouseEvent>                                      // for QMouseEvent
#include <QSizeF>                                           // for QSizeF
#include <QSizePolicy>                                      // for QSizePolicy, QSizePolicy::Fixed
#include <QtCore/qcoreevent.h>                              // for QEvent (ptr only), QEvent::Mo...
#include <QtCore/qnamespace.h>                              // for AlignLeft, ElideRight, Preven...

class QHBoxLayout;

#include <warn/pop>

namespace inviwo {

EditableLabelQt::EditableLabelQt(PropertyWidgetQt* parent, const std::string& text,
                                 bool shortenText)
    : QWidget(parent)
    , label_{new QLabel(this)}
    , lineEdit_{nullptr}
    , text_(text)
    , property_(nullptr)
    , propertyWidget_(parent)
    , shortenText_(shortenText) {

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);

    updateLabel(text_);

    hLayout->addWidget(label_);
    setLayout(hLayout);

    QSizePolicy labelPol = sizePolicy();
    labelPol.setHorizontalStretch(1);
    labelPol.setVerticalPolicy(QSizePolicy::Fixed);
    setSizePolicy(labelPol);
    label_->setSizePolicy(labelPol);

    setContextMenuPolicy(Qt::PreventContextMenu);
}

EditableLabelQt::EditableLabelQt(PropertyWidgetQt* parent, Property* property, bool shortenText)
    : EditableLabelQt(parent, property->getDisplayName(), shortenText) {
    property_ = property;
    property->addObserver(this);
}

std::string EditableLabelQt::getText() { return text_; }

QLineEdit* EditableLabelQt::getLineEdit() {
    if (!lineEdit_) {
        lineEdit_ = new QLineEdit(this);
        lineEdit_->hide();
        lineEdit_->setAlignment(Qt::AlignLeft);
        lineEdit_->setContentsMargins(0, 0, 0, 0);
        setFocusProxy(lineEdit_);
        label_->setFocusProxy(lineEdit_);
        lineEdit_->setSizePolicy(sizePolicy());
        layout()->addWidget(lineEdit_);

        connect(lineEdit_, &QLineEdit::editingFinished, this, [&]() {
            text_ = lineEdit_->text().toStdString();
            updateLabel(text_);

            lineEdit_->hide();
            label_->show();

            if (property_) property_->setDisplayName(text_);

            emit textChanged();
        });
    }
    return lineEdit_;
}

void EditableLabelQt::edit() {
    auto le = getLineEdit();
    le->setText(QString::fromStdString(text_));
    label_->hide();
    le->show();
    le->setCursorPosition(static_cast<int>(le->text().size()));
    le->setFocusProxy(nullptr);
    le->setFocus();
}

QSize EditableLabelQt::sizeHint() const { return utilqt::emToPx(this, QSizeF(2.0, 2.0)); }
QSize EditableLabelQt::minimumSizeHint() const { return sizeHint(); }

void EditableLabelQt::resizeEvent(QResizeEvent* event) {
    updateLabel(text_);
    QWidget::resizeEvent(event);
}

void EditableLabelQt::mouseDoubleClickEvent(QMouseEvent*) { edit(); }

void EditableLabelQt::setText(const std::string& txt) {
    text_ = txt;
    updateLabel(text_);
}

QString EditableLabelQt::shortenText(const std::string& text) {
    QFontMetrics fm = label_->fontMetrics();
    return fm.elidedText(QString::fromStdString(text), Qt::ElideRight, width());
}

void EditableLabelQt::updateLabel(const std::string& text) {
    label_->setText(shortenText_ ? shortenText(text) : QString::fromStdString(text));
}

void EditableLabelQt::onSetDisplayName(Property*, const std::string& displayName) {
    text_ = displayName;
    updateLabel(text_);
}

void EditableLabelQt::setShortenText(bool shorten) {
    shortenText_ = shorten;
    updateLabel(text_);
}

bool EditableLabelQt::event(QEvent* event) {
    if (event->type() == QEvent::MouseButtonRelease) {
        auto mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::RightButton) {
            if (auto menu = propertyWidget_->getContextMenu()) {
                auto renameAction = menu->addAction(tr("&Rename"));
                connect(renameAction, &QAction::triggered, this, &EditableLabelQt::edit);
                menu->exec(mouseEvent->globalPos());
                mouseEvent->accept();
                return true;
            }
        }
    }
    return QWidget::event(event);
}

}  // namespace inviwo
