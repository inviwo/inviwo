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

#include <modules/qtwidgets/properties/buttonpropertywidgetqt.h>

#include <inviwo/core/properties/buttonproperty.h>          // for ButtonProperty
#include <modules/qtwidgets/inviwoqtutils.h>                // for fromQString, toQString
#include <modules/qtwidgets/properties/propertywidgetqt.h>  // for PropertyWidgetQt

#include <QAction>       // for QAction
#include <QHBoxLayout>   // for QHBoxLayout
#include <QInputDialog>  // for QInputDialog
#include <QLineEdit>     // for QLineEdit, QLineEdit::Normal
#include <QMenu>         // for QMenu
#include <QPushButton>   // for QPushButton
#include <QString>       // for QString

class QHBoxLayout;

namespace inviwo {

class Property;

ButtonPropertyWidgetQt::ButtonPropertyWidgetQt(ButtonProperty* property)
    : PropertyWidgetQt(property), property_(property) {

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);
    button_ = new QPushButton();
    button_->setText(QString::fromStdString(property_->getDisplayName()));
    connect(button_, &QPushButton::released, this, [&]() {
        if (!property_->getReadOnly()) {
            try {
                property_->pressButton();
            } catch (const Exception& e) {
                log::exception(e);
            }
        }
    });
    button_->setFocusPolicy(Qt::StrongFocus);
    setFocusPolicy(button_->focusPolicy());
    setFocusProxy(button_);

    hLayout->addWidget(button_);
    setLayout(hLayout);
    updateFromProperty();
}

void ButtonPropertyWidgetQt::onSetDisplayName(Property*, const std::string& displayName) {
    button_->setText(QString::fromStdString(displayName));
}

void ButtonPropertyWidgetQt::updateFromProperty() {
    button_->setText(QString::fromStdString(property_->getDisplayName()));
}

QPushButton* ButtonPropertyWidgetQt::getButton() { return button_; }

std::unique_ptr<QMenu> ButtonPropertyWidgetQt::getContextMenu() {
    auto menu = PropertyWidgetQt::getContextMenu();

    auto renameAction = menu->addAction(tr("&Rename"));
    connect(renameAction, &QAction::triggered, this, [this]() {
        bool ok;
        const auto displayname = utilqt::toQString(property_->getDisplayName());
        const auto name = utilqt::fromQString(QInputDialog::getText(
            nullptr, "Rename", "Name of Property", QLineEdit::Normal, displayname, &ok));
        if (ok) {
            property_->setDisplayName(name);
        }
    });
    return menu;
}

}  // namespace inviwo
