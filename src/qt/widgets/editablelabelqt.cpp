/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include <inviwo/qt/widgets/editablelabelqt.h>

#include <QFontMetrics>

namespace inviwo {

EditableLabelQt::EditableLabelQt(QWidget* parent, std::string text, bool shortenText)
    : QWidget(parent)
    , text_(text)
    , propertyWidget_(NULL)
    , contextMenu_(NULL)
    , shortenText_(shortenText) {

    generateWidget();
}

EditableLabelQt::EditableLabelQt(PropertyWidgetQt* parent, std::string text, bool shortenText)
    : QWidget(parent)
    , text_(text)
    , propertyWidget_(parent)
    , contextMenu_(NULL)
    , shortenText_(shortenText) {

    generateWidget();
}

void EditableLabelQt::generateWidget() {
    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);
    label_ = new QLabel(this);

    if (shortenText_)
        label_->setText(QString::fromStdString(shortenText()));
    else
        label_->setText(QString::fromStdString(text_));

    lineEdit_ = new QLineEdit(this);
    hLayout->addWidget(lineEdit_);
    lineEdit_->hide();
    lineEdit_->setAlignment(Qt::AlignLeft);
    lineEdit_->setContentsMargins(0,0,0,0);
    hLayout->addWidget(label_);
    setLayout(hLayout);
    
    QSizePolicy labelPol = this->sizePolicy();
    labelPol.setHorizontalStretch(1);
    labelPol.setVerticalPolicy(QSizePolicy::Fixed);
    this->setSizePolicy(labelPol);
    label_->setSizePolicy(labelPol);
    lineEdit_->setSizePolicy(labelPol);

    label_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(label_,
            SIGNAL(customContextMenuRequested(const QPoint&)),
            this,
            SLOT(showContextMenu(const QPoint&)));

    connect(lineEdit_,
            SIGNAL(editingFinished()),
            this,
            SLOT(finishEditing()));
}

void EditableLabelQt::edit() {
    label_->hide();
    lineEdit_->setText(QString::fromStdString(text_));
    lineEdit_->show();
    lineEdit_->setFocus();
    lineEdit_->selectAll();
}

QSize EditableLabelQt::sizeHint() const { return QSize(18, 18); }
QSize EditableLabelQt::minimumSizeHint() const { return sizeHint(); }

void EditableLabelQt::resizeEvent(QResizeEvent *event) {
    if (shortenText_){
        label_->setText(QString::fromStdString(shortenText()));
    }
    QWidget::resizeEvent(event);
}


void EditableLabelQt::mouseDoubleClickEvent(QMouseEvent* e) {
    edit();
}

void EditableLabelQt::finishEditing() {
    lineEdit_->hide();
    text_ = lineEdit_->text().toLocal8Bit().constData();

    if (shortenText_){
        label_->setText(QString::fromStdString(shortenText()));
    } else {
        label_->setText(QString::fromStdString(text_));
    }

    label_->show();
    emit textChanged();
}

void EditableLabelQt::setText(std::string txt) {
    text_ = txt;
    edit();
    finishEditing();
}

void EditableLabelQt::showContextMenu(const QPoint& pos) {
    if (!contextMenu_) {
        contextMenu_ = new QMenu(this);

        if (propertyWidget_) {
            contextMenu_->addActions(propertyWidget_->getContextMenu()->actions());
        }

        renameAction_ = new QAction(tr("&Rename"), this);
        contextMenu_->addAction(renameAction_);
        connect(renameAction_, SIGNAL(triggered()), this, SLOT(edit()));
    }

    contextMenu_->exec(label_->mapToGlobal(pos));
}

std::string EditableLabelQt::shortenText() {
    QFontMetrics fm = label_->fontMetrics();
    return fm.elidedText(QString::fromStdString(text_), Qt::ElideRight, width()).toStdString();
}

void EditableLabelQt::setShortenText(bool shorten) {
    shortenText_ = shorten;
    if (shortenText_) {
        label_->setText(QString::fromStdString(shortenText()));
    } else {
        label_->setText(QString::fromStdString(text_));
    }
}

} //namespace