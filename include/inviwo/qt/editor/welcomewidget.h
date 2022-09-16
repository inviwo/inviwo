/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2022 Inviwo Foundation
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

#include <inviwo/qt/editor/inviwoqteditordefine.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QModelIndex>
#include <QSplitter>
#include <QStringList>
#include <QTextEdit>
#include <warn/pop>

class QTabWidget;
class QToolButton;
class QLineEdit;
class QTextEdit;
class QScrollArea;
class QSortFilterProxyModel;
class QSvgWidget;

namespace inviwo {

class HorisontalCollipsable;
class WorkspaceTreeModel;
class WorkspaceTreeView;
class WorkspaceGridView;
class InviwoApplication;
class ChangeLog;
class WorkspaceFilter;

class IVW_QTEDITOR_API WelcomeWidget : public QSplitter {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    WelcomeWidget(InviwoApplication* app, QWidget* parent);
    virtual ~WelcomeWidget() = default;

    void updateRecentWorkspaces(const QStringList& list);
    void enableRestoreButton(bool hasRestoreWorkspace);

signals:
    void loadWorkspace(const QString& filename, bool isExample);
    void appendWorkspace(const QString& filename);
    void newWorkspace();
    void openWorkspace();
    void restoreWorkspace();

protected:
    virtual void showEvent(QShowEvent* event) override;
    virtual void hideEvent(QHideEvent* event) override;

    virtual void keyPressEvent(QKeyEvent* event) override;

private:
    WelcomeWidget& setSetting(const QString& key, const QVariant& value);
    QVariant getSetting(const QString& key, const QVariant& defaultValue = QVariant()) const;

    void updateDetails(const QModelIndex& index);
    static QModelIndex findFirstLeaf(QAbstractItemModel* model, QModelIndex parent = QModelIndex());

    void selectFirstLeaf();

    void expandTreeView() const;

    void onChangelogCollapseChanged(bool collapse);

    InviwoApplication* app_;

    WorkspaceTreeModel* model_;
    WorkspaceFilter* filterModel_;
    WorkspaceTreeView* workspaceTreeView_;
    WorkspaceGridView* workspaceGridView_;

    QFrame* rightColumn_;
    HorisontalCollipsable* collapsibleChangeLog_;

    QLineEdit* filterLineEdit_;
    QSvgWidget* dragAndDrop_;
    QTextEdit* details_;
    ChangeLog* changelog_;
    QToolButton* loadWorkspaceBtn_;
    QToolButton* appendWorkspaceBtn_;
    QToolButton* restoreButton_;
};

}  // namespace inviwo
