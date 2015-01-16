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

#include <inviwo/qt/widgets/properties/transferfunctionpropertywidgetqt.h>
#include <inviwo/qt/widgets/properties/collapsiblegroupboxwidgetqt.h>
#include <inviwo/qt/widgets/inviwoapplicationqt.h>
#include <inviwo/qt/widgets/editablelabelqt.h>
#include <inviwo/qt/widgets/properties/transferfunctionpropertydialog.h>
#include <QHBoxLayout>

namespace inviwo {

TransferFunctionPropertyWidgetQt::TransferFunctionPropertyWidgetQt(TransferFunctionProperty* property) 
    : PropertyWidgetQt(property), transferFunctionDialog_(NULL) {
    generateWidget();
    updateFromProperty();
}

TransferFunctionPropertyWidgetQt::~TransferFunctionPropertyWidgetQt() {
    transferFunctionDialog_->hide();
    setEditorWidget(NULL);
    delete transferFunctionDialog_;
    delete btnOpenTF_;
}

void TransferFunctionPropertyWidgetQt::generateWidget() {
    InviwoApplicationQt* app = dynamic_cast<InviwoApplicationQt*>(InviwoApplication::getPtr());
    transferFunctionDialog_ = new TransferFunctionPropertyDialog(
        static_cast<TransferFunctionProperty*>(property_), app->getMainWindow());
    setEditorWidget(transferFunctionDialog_);
    // notify the transfer function dialog that the volume with the histogram is already there
    // TODO: Make sure that this work without notify. Can we do this in another way? It seems very
    // weird...
    transferFunctionDialog_->getEditorView()->onTransferFunctionChange();
    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(7);

    btnOpenTF_ = new IvwPushButton(this);

    label_ = new EditableLabelQt(this, property_->getDisplayName());
    hLayout->addWidget(label_);
    connect(label_, SIGNAL(textChanged()), this, SLOT(setPropertyDisplayName()));

    connect(btnOpenTF_, SIGNAL(clicked()), this, SLOT(openTransferFunctionDialog()));

    btnOpenTF_->setEnabled(!property_->getReadOnly());

    {
        QWidget* widget = new QWidget(this);
        QSizePolicy sliderPol = widget->sizePolicy();
        sliderPol.setHorizontalStretch(3);
        widget->setSizePolicy(sliderPol);
        QGridLayout* vLayout = new QGridLayout();
        widget->setLayout(vLayout);
        vLayout->setContentsMargins(0, 0, 0, 0);
        vLayout->setSpacing(0);
        
        vLayout->addWidget(btnOpenTF_);
        hLayout->addWidget(widget);
    }

    setLayout(hLayout);
    updateFromProperty();
    // initializes position, visibility,size of the widget from meta data
    initializeEditorWidgetsMetaData();

    QSizePolicy sp = sizePolicy();
    sp.setVerticalPolicy(QSizePolicy::Fixed);
    setSizePolicy(sp);
}

void TransferFunctionPropertyWidgetQt::updateFromProperty() {
    QSize gradientSize = btnOpenTF_->size() - QSize(2, 2);

    QLinearGradient* gradient = transferFunctionDialog_->getTFGradient();
    gradient->setFinalStop(gradientSize.width(), 0);
    QPixmap tfPixmap(gradientSize);
    QPainter tfPainter(&tfPixmap);
    QPixmap checkerBoard(10, 10);
    QPainter checkerBoardPainter(&checkerBoard);
    checkerBoardPainter.fillRect(0, 0, 5, 5, Qt::lightGray);
    checkerBoardPainter.fillRect(5, 0, 5, 5, Qt::darkGray);
    checkerBoardPainter.fillRect(0, 5, 5, 5, Qt::darkGray);
    checkerBoardPainter.fillRect(5, 5, 5, 5, Qt::lightGray);
    checkerBoardPainter.end();
    tfPainter.fillRect(0, 0, gradientSize.width(), gradientSize.height(), QBrush(checkerBoard));
    tfPainter.fillRect(0, 0, gradientSize.width(), gradientSize.height(), *gradient);
    // Cast for convenience (safe to static cast since we know that property_ is a
    // TransferFunctionProperty)
    TransferFunctionProperty* tfProperty = static_cast<TransferFunctionProperty*>(property_);
    // draw masking indicators
    if (tfProperty->getMask().x > 0.0f) {
        tfPainter.fillRect(0, 0, static_cast<int>(tfProperty->getMask().x * gradientSize.width()),
                           btnOpenTF_->height(), QColor(25, 25, 25, 100));

        tfPainter.drawLine(static_cast<int>(tfProperty->getMask().x * gradientSize.width()), 0,
                           static_cast<int>(tfProperty->getMask().x * gradientSize.width()),
                           btnOpenTF_->height());
    }

    if (tfProperty->getMask().y < 1.0f) {
        tfPainter.fillRect(
            static_cast<int>(tfProperty->getMask().y * gradientSize.width()), 0,
            static_cast<int>((1.0f - tfProperty->getMask().y) * gradientSize.width()) + 1,
            btnOpenTF_->height(), QColor(25, 25, 25, 150));

        tfPainter.drawLine(static_cast<int>(tfProperty->getMask().y * gradientSize.width()), 0,
                           static_cast<int>(tfProperty->getMask().y * gradientSize.width()),
                           btnOpenTF_->height());
    }

    btnOpenTF_->setIcon(tfPixmap);
    btnOpenTF_->setIconSize(gradientSize);

    this->setDisabled(property_->getReadOnly());
}

void TransferFunctionPropertyWidgetQt::setPropertyValue() {}

void TransferFunctionPropertyWidgetQt::openTransferFunctionDialog() {
    transferFunctionDialog_->setVisible(true);
}

void TransferFunctionPropertyWidgetQt::setPropertyDisplayName() {
    property_->setDisplayName(label_->getText());
}

// Needed to resize the graident preview.
void TransferFunctionPropertyWidgetQt::resizeEvent(QResizeEvent* event) {
    updateFromProperty();
    PropertyWidgetQt::resizeEvent(event);
}

}//namespace