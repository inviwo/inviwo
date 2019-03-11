/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#include <inviwo/qt/editor/inviwoeditmenu.h>

#include <inviwo/qt/editor/inviwomainwindow.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QWidget>
#include <QObject>
#include <QApplication>
#include <warn/pop>

namespace inviwo {

InviwoEditMenu::InviwoEditMenu(InviwoMainWindow* win) : QMenu(tr("&Edit"), win) {
    {
        auto cutAction = addAction(QIcon(":/svgicons/edit-cut.svg"), tr("Cu&t"));
        actions_[MenuItemType::cut] = cutAction;
        cutAction->setShortcut(QKeySequence::Cut);
    }
    {
        auto copyAction = addAction(QIcon(":/svgicons/edit-copy.svg"), tr("&Copy"));
        actions_[MenuItemType::copy] = copyAction;
        copyAction->setShortcut(QKeySequence::Copy);
    }
    {
        auto pasteAction = addAction(QIcon(":/svgicons/edit-paste.svg"), tr("&Paste"));
        actions_[MenuItemType::paste] = pasteAction;
        pasteAction->setShortcut(QKeySequence::Paste);
    }
    {
        auto deleteAction = addAction(QIcon(":/svgicons/edit-delete.svg"), tr("&Delete"));
        actions_[MenuItemType::del] = deleteAction;
        deleteAction->setShortcuts(QList<QKeySequence>(
            {QKeySequence::Delete, QKeySequence(Qt::ControlModifier + Qt::Key_Backspace)}));
    }
    {
        auto selectAllAction = addAction(QIcon(":/svgicons/edit-selectall.svg"), tr("&Select All"));
        actions_[MenuItemType::select] = selectAllAction;
        selectAllAction->setShortcut(QKeySequence::SelectAll);
    }

    for (auto& action : actions_) {
        action.second->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        action.second->setEnabled(true);
        connect(action.second, &QAction::triggered, this, [this, type = action.first]() {
            if (auto item = getFocusItem()) {
                item->invoke(type);
            }
        });
    }

    // enable the actions currently supported by the item in focus
    connect(this, &QMenu::aboutToShow, this, [this]() {
        if (auto item = getFocusItem()) {
            for (auto& action : actions_) {
                action.second->setEnabled(item->enabled(action.first));
            }
        } else {
            for (auto& action : actions_) {
                action.second->setEnabled(false);
            }
        }
    });

    // need to enable all for potential shortcut use to function
    connect(this, &QMenu::aboutToHide, this, [this]() {
        for (auto& action : actions_) {
            action.second->setEnabled(true);
        }
    });
}

std::shared_ptr<MenuItem> InviwoEditMenu::getFocusItem() {
    QObject* focus = QApplication::focusWidget();
    while (focus) {
        auto it = items_.find(focus);
        if (it != items_.end()) {
            if (auto item = it->second.lock()) {
                lastItem_ = item;
                return item;
            } else {
                items_.erase(it);
            }
        } else {
            focus = focus->parent();
        }
    }

    if (auto item = lastItem_.lock()) {
        return item;
    }

    return nullptr;
}

std::shared_ptr<MenuItem> InviwoEditMenu::registerItem(std::shared_ptr<MenuItem> item) {
    items_[item->owner] = item;
    if (auto w = qobject_cast<QWidget*>(item->owner)) {
        for (auto& action : actions_) {
            w->addAction(action.second);
        }
    }
    return item;
}

MenuItem::MenuItem(QObject* owner_, std::function<bool(MenuItemType)> enabled_,
                   std::function<void(MenuItemType)> invoke_)
    : owner{owner_}, enabled{std::move(enabled_)}, invoke(std::move(invoke_)) {}

}  // namespace inviwo
