/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2022 Inviwo Foundation
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

#include <modules/qtwidgets/properties/isovaluepropertywidgetqt.h>

#include <inviwo/core/datastructures/isovaluecollection.h>    // for IsoValueCollection
#include <inviwo/core/network/networklock.h>                  // for NetworkLock
#include <inviwo/core/properties/isovalueproperty.h>          // for IsoValueProperty
#include <inviwo/core/properties/property.h>                  // for Property
#include <modules/qtwidgets/editablelabelqt.h>                // for EditableLabelQt
#include <modules/qtwidgets/inviwowidgetsqt.h>                // for IvwPushButton
#include <modules/qtwidgets/properties/propertywidgetqt.h>    // for PropertyWidgetQt
#include <modules/qtwidgets/properties/tfpropertywidgetqt.h>  // for TFPushButton
#include <modules/qtwidgets/tf/tfpropertydialog.h>            // for TFPropertyDialog
#include <modules/qtwidgets/tf/tfutils.h>                     // for exportToFile, importFromFile

#include <warn/push>
#include <warn/ignore/all>
#include <QAction>                                            // for QAction
#include <QHBoxLayout>                                        // for QHBoxLayout
#include <QMenu>                                              // for QMenu
#include <QSizePolicy>                                        // for QSizePolicy, QSizePolicy::F...

class QHBoxLayout;

#include <warn/pop>

namespace inviwo {

IsoValuePropertyWidgetQt::IsoValuePropertyWidgetQt(IsoValueProperty* property)
    : PropertyWidgetQt(property)
    , label_{new EditableLabelQt(this, property_)}
    , btnOpenTF_{new TFPushButton(property, this)} {

    setFocusPolicy(btnOpenTF_->focusPolicy());
    setFocusProxy(btnOpenTF_);

    QHBoxLayout* hLayout = new QHBoxLayout;
    setSpacingAndMargins(hLayout);

    hLayout->addWidget(label_);
    hLayout->addWidget(btnOpenTF_);

    setLayout(hLayout);
    updateFromProperty();

    connect(btnOpenTF_, &IvwPushButton::clicked, [this]() {
        if (!tfDialog_) {
            tfDialog_ =
                std::make_unique<TFPropertyDialog>(static_cast<IsoValueProperty*>(property_));
            tfDialog_->setVisible(true);
        } else {
            tfDialog_->setVisible(!tfDialog_->isVisible());
        }
    });

    QSizePolicy sp = sizePolicy();
    sp.setVerticalPolicy(QSizePolicy::Fixed);
    setSizePolicy(sp);
}

TFPropertyDialog* IsoValuePropertyWidgetQt::getEditorWidget() const { return tfDialog_.get(); }

bool IsoValuePropertyWidgetQt::hasEditorWidget() const { return tfDialog_ != nullptr; }

void IsoValuePropertyWidgetQt::updateFromProperty() { btnOpenTF_->updateFromProperty(); }

void IsoValuePropertyWidgetQt::setReadOnly(bool readonly) { label_->setDisabled(readonly); }

std::unique_ptr<QMenu> IsoValuePropertyWidgetQt::getContextMenu() {
    auto menu = PropertyWidgetQt::getContextMenu();

    menu->addSeparator();

    auto clearTF = menu->addAction("&Clear Isovalues");
    clearTF->setEnabled(!property_->getReadOnly());

    connect(clearTF, &QAction::triggered, this, [this]() {
        NetworkLock lock(property_);
        static_cast<IsoValueProperty*>(property_)->get().clear();
    });

    auto importIso = menu->addAction("&Import Isovalues...");
    auto exportIso = menu->addAction("&Export Isovalues...");
    importIso->setEnabled(!property_->getReadOnly());
    connect(importIso, &QAction::triggered, this, [this]() {
        util::importFromFile(static_cast<IsoValueProperty*>(property_)->get(), this);
    });
    connect(exportIso, &QAction::triggered, this, [this]() {
        util::exportToFile(static_cast<IsoValueProperty*>(property_)->get(), this);
    });

    return menu;
}
}  // namespace inviwo
