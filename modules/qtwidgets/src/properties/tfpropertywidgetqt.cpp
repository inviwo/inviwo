/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#include <modules/qtwidgets/properties/tfpropertywidgetqt.h>
#include <modules/qtwidgets/properties/collapsiblegroupboxwidgetqt.h>
#include <modules/qtwidgets/editablelabelqt.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/tf/tfutils.h>
#include <inviwo/core/properties/transferfunctionproperty.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QHBoxLayout>
#include <QWidget>
#include <warn/pop>

namespace inviwo {

TFPropertyWidgetQt::TFPropertyWidgetQt(TransferFunctionProperty* property)
    : PropertyWidgetQt(property)
    , label_{new EditableLabelQt(this, property)}
    , btnOpenTF_{new TFPushButton(property, this)} {

    setFocusPolicy(btnOpenTF_->focusPolicy());
    setFocusProxy(btnOpenTF_);

    QHBoxLayout* hLayout = new QHBoxLayout();
    setSpacingAndMargins(hLayout);

    hLayout->addWidget(label_);

    connect(btnOpenTF_, &TFPushButton::clicked, [this]() {
        if (!transferFunctionDialog_) {
            transferFunctionDialog_ = std::make_unique<TFPropertyDialog>(
                static_cast<TransferFunctionProperty*>(property_));
            transferFunctionDialog_->setVisible(true);
        } else {
            transferFunctionDialog_->setVisible(!transferFunctionDialog_->isVisible());
        }
    });

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

    QSizePolicy sp = sizePolicy();
    sp.setVerticalPolicy(QSizePolicy::Fixed);
    setSizePolicy(sp);
}

TFPropertyWidgetQt::~TFPropertyWidgetQt() {
    if (transferFunctionDialog_) transferFunctionDialog_->hide();
}

void TFPropertyWidgetQt::updateFromProperty() { btnOpenTF_->updateFromProperty(); }

TFPropertyDialog* TFPropertyWidgetQt::getEditorWidget() const {
    return transferFunctionDialog_.get();
}

bool TFPropertyWidgetQt::hasEditorWidget() const { return transferFunctionDialog_ != nullptr; }

void TFPropertyWidgetQt::setReadOnly(bool readonly) { label_->setDisabled(readonly); }

std::unique_ptr<QMenu> TFPropertyWidgetQt::getContextMenu() {
    auto menu = PropertyWidgetQt::getContextMenu();

    menu->addSeparator();

    util::addTFPresetsMenu(this, menu.get(), static_cast<TransferFunctionProperty*>(property_));

    auto transformMenu = menu->addMenu("TF &Transform");

    auto flip = transformMenu->addAction("&Horizontal Flip");
    auto interpolate = transformMenu->addAction("&Interpolate Alpha");
    auto equalize = transformMenu->addAction("&Equalize Alpha");

    connect(flip, &QAction::triggered, this, [this]() {
        NetworkLock lock(property_);
        auto p = static_cast<TransferFunctionProperty*>(property_);
        p->get().flipPositions();
    });
    connect(interpolate, &QAction::triggered, this, [this]() {
        NetworkLock lock(property_);
        auto p = static_cast<TransferFunctionProperty*>(property_);
        p->get().interpolateAlpha();
    });
    connect(equalize, &QAction::triggered, this, [this]() {
        NetworkLock lock(property_);
        auto p = static_cast<TransferFunctionProperty*>(property_);
        p->get().equalizeAlpha();
    });

    auto clearTF = menu->addAction("&Clear TF");
    clearTF->setEnabled(!property_->getReadOnly());

    connect(clearTF, &QAction::triggered, this, [this]() {
        NetworkLock lock(property_);
        static_cast<TransferFunctionProperty*>(property_)->get().clear();
    });

    auto importTF = menu->addAction("&Import TF...");
    auto exportTF = menu->addAction("&Export TF...");
    importTF->setEnabled(!property_->getReadOnly());
    connect(importTF, &QAction::triggered, this, [this]() {
        util::importFromFile(static_cast<TransferFunctionProperty*>(property_)->get(), this);
    });
    connect(exportTF, &QAction::triggered, this, [this]() {
        util::exportToFile(static_cast<TransferFunctionProperty*>(property_)->get(), this);
    });

    return menu;
}

TFPushButton::TFPushButton(TransferFunctionProperty* property, QWidget* parent)
    : IvwPushButton(parent)
    , propertyPtr_(std::make_unique<util::TFPropertyModel<TransferFunctionProperty*>>(property)) {}

TFPushButton::TFPushButton(IsoValueProperty* property, QWidget* parent)
    : IvwPushButton(parent)
    , propertyPtr_(std::make_unique<util::TFPropertyModel<IsoValueProperty*>>(property)) {}

TFPushButton::TFPushButton(IsoTFProperty* property, QWidget* parent)
    : IvwPushButton(parent)
    , propertyPtr_(std::make_unique<util::TFPropertyModel<IsoTFProperty*>>(property)) {}

void TFPushButton::updateFromProperty() {
    const QSize size = this->size() - QSize(2, 2);

    setIcon(utilqt::toQPixmap(*propertyPtr_, size));
    setIconSize(size);
}

void TFPushButton::resizeEvent(QResizeEvent* event) {
    updateFromProperty();
    IvwPushButton::resizeEvent(event);
}

}  // namespace inviwo
