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

#ifndef IVW_INVIWOEDITMENU_H
#define IVW_INVIWOEDITMENU_H

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <inviwo/core/common/inviwo.h>

#include <functional>
#include <unordered_map>

#include <warn/push>
#include <warn/ignore/all>
#include <QMenu>
#include <warn/pop>

class QAction;
class QWidget;
class QMenu;

namespace inviwo {

class InviwoMainWindow;

enum class MenuItemType { cut, copy, paste, del, select };

class IVW_QTEDITOR_API MenuItem {
public:
    MenuItem(QObject* owner, std::function<bool(MenuItemType)> enabled,
             std::function<void(MenuItemType)> invoke);
    QObject* owner;
    std::function<bool(MenuItemType)> enabled;
    std::function<void(MenuItemType)> invoke;
};

/**
 * Manage menu entries for the main window.
 * Map the action to the focused widget
 */
class IVW_QTEDITOR_API InviwoEditMenu : public QMenu {
public:
    InviwoEditMenu(InviwoMainWindow* win);
    virtual ~InviwoEditMenu() = default;

    std::shared_ptr<MenuItem> registerItem(std::shared_ptr<MenuItem> item);

private:
    std::shared_ptr<MenuItem> getFocusItem();

    std::unordered_map<QObject*, std::weak_ptr<MenuItem>> items_;
    std::map<MenuItemType, QAction*> actions_;
    std::weak_ptr<MenuItem> lastItem_;
};

}  // namespace inviwo

#endif  // IVW_INVIWOEDITMENU_H
