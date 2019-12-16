/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <modules/qtwidgets/properties/listpropertywidgetqt.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/inviwowidgetsqt.h>

#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/listproperty.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/exception.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QMenu>
#include <QAction>
#include <QToolButton>
#include <QHBoxLayout>
#include <warn/pop>

namespace inviwo {

ListPropertyWidgetQt::ListPropertyWidgetQt(ListProperty* property)
    : CompositePropertyWidgetQt(property), listProperty_(property) {

    if (!listProperty_) {
        throw Exception("ListPropertyWidgetQt got a null ListProperty", IVW_CONTEXT);
    }

    setShowIfEmpty(true);
    setEmptyLabelString("No list entries");

    auto headerlayout = dynamic_cast<QHBoxLayout*>(layout()->itemAt(0)->layout());
    if (!headerlayout) {
        throw Exception("CompositePropertyWidget has no header layout.", IVW_CONTEXT);
    }

    // inject "add" button in the header of the composite property for adding list elements
    addItemButton_ = new QToolButton(this);
    addItemButton_->setObjectName("addListItemButton");
    addItemButton_->setToolTip("Add list element");

    setFocusPolicy(addItemButton_->focusPolicy());
    setFocusProxy(addItemButton_);

    connect(addItemButton_, &QToolButton::clicked, this, [&]() {
        if (listProperty_ && listProperty_->getPrefabCount() > 0) {
            if (listProperty_->getPrefabCount() == 1) {
                addNewItem(0);
            } else {
                // show context menu since there exist more than one prefab in the list property
                QMenu m;
                for (auto&& item : util::enumerate(listProperty_->getPrefabs())) {
                    auto addAction =
                        m.addAction(utilqt::toQString(item.second()->getDisplayName()));
                    addAction->setData(static_cast<uint>(item.first()));
                }

                if (auto selectedAction = m.exec(QCursor::pos())) {
                    addNewItem(selectedAction->data().toUInt());
                }
            }
        }
    });

    addItemButton_->setVisible(
        static_cast<bool>(listProperty_->getUIFlags() & ListPropertyUIFlag::Add));

    headerlayout->insertWidget(headerlayout->count() - 2, addItemButton_);
}

void ListPropertyWidgetQt::updateFromProperty() {
    CompositePropertyWidgetQt::updateFromProperty();
    addItemButton_->setEnabled(canAddElements());
}

bool ListPropertyWidgetQt::isChildRemovable() const {
    return static_cast<bool>(listProperty_->getUIFlags() & ListPropertyUIFlag::Remove);
}

std::unique_ptr<QMenu> ListPropertyWidgetQt::getContextMenu() {
    auto menu = CompositePropertyWidgetQt::getContextMenu();

    menu->addSeparator();

    if (listProperty_->getUIFlags() & ListPropertyUIFlag::Add) {
        if (listProperty_->getPrefabCount() == 1) {
            auto addAction = menu->addAction(
                utilqt::toQString("Add " + listProperty_->getPrefabs().front()->getDisplayName()));
            connect(addAction, &QAction::triggered, this, [this]() { addNewItem(0); });
        } else {
            auto addItemMenu = menu->addMenu("&Add Item");
            if (listProperty_->getPrefabCount() > 0) {
                for (auto&& item : util::enumerate(listProperty_->getPrefabs())) {
                    auto addAction =
                        addItemMenu->addAction(utilqt::toQString(item.second()->getDisplayName()));
                    connect(addAction, &QAction::triggered, this,
                            [this, index = item.first()]() { addNewItem(index); });
                }
            } else {  // no prefab objects available
                auto action = addItemMenu->addAction("No templates available");
                action->setEnabled(false);
            }

            // disable add item menu if list property is already fully filled
            addItemMenu->setEnabled(canAddElements());
        }
    }

    if (listProperty_->getUIFlags() & ListPropertyUIFlag::Remove) {
        auto clearAction = menu->addAction(tr("&Clear Items"));
        connect(clearAction, &QAction::triggered, this, [&]() {
            listProperty_->setInitiatingWidget(this);
            listProperty_->clear();
            listProperty_->clearInitiatingWidget();
        });
    }

    menu->addSeparator();

    return menu;
}

void ListPropertyWidgetQt::addNewItem(size_t index) {
    listProperty_->setInitiatingWidget(this);
    listProperty_->constructProperty(index);
    listProperty_->clearInitiatingWidget();
}

bool ListPropertyWidgetQt::canAddElements() const {
    return (listProperty_->getMaxNumberOfElements() == 0) ||
           (listProperty_->size() < listProperty_->getMaxNumberOfElements());
}

}  // namespace inviwo
