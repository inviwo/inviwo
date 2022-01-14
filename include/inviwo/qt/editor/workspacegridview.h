/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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
#include <inviwo/qt/editor/filetreemodel.h>

#include <warn/push>
#include <warn/ignore/all>

#include <QFileSystemModel>
#include <QtGlobal>
#include <QListView>
#include <QIcon>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include <QPersistentModelIndex>

#include <warn/pop>

class QVBoxLayout;
class QLabel;
class QItemSelectionModel;

namespace inviwo {

class InviwoApplication;
class TreeItem;
class FileTreeModel;
/**
 * \brief ListView that assumes icon grid layout and ensures a proper sizeHint for the widget.
 * The custom listview can also handle filtering when setRootIndex is used.
 * setRootIndex can be used to start the list from a (hiearchical) item in the model.
 * However, QListView resets the root index when the index becomes invalid through filtering using QSortFilterProxyModel.
 * The filtering workaround is based on the answers here: https://stackoverflow.com/questions/70112321/qt-rootindex-gets-reset-each-time-qsortfilterproxymodelinvalidatefilter-is-c
 */
class FixedSizeListView : public QListView
{
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    explicit FixedSizeListView(QWidget *parent = nullptr)
        : QListView(parent) {}
    
    void setModel(QAbstractItemModel *model) override;
    /*
     * @return the content width and a height that depends on how many items that fit in a row.
     */
    QSize sizeHint() const override;
    
    QSize minimumSizeHint() const override {
        return sizeHint();
    }
     
public slots:
    virtual void setRootIndex(const QModelIndex& rootIndex) override;
    void checkRootIndex();
private:
    QSortFilterProxyModel* model_ = nullptr;
    QPersistentModelIndex sourceRootIndex_;
     
};


/**
 * \brief Displays recently used workspaces, example workspaces and regression test workspaces in a grid list.
 * Each workspace is represented by the first avaialble canvas image and its filename.
 */
class IVW_QTEDITOR_API WorkspaceGridView  : public QWidget {
#include <warn/push>
#include <warn/ignore/all>
    Q_OBJECT
#include <warn/pop>
public:
    explicit WorkspaceGridView(FileTreeModel* model, QSortFilterProxyModel* workspaceProxyModel,
                               QItemSelectionModel* selectionModel, QWidget* parent = nullptr);
    virtual ~WorkspaceGridView() = default;

signals:
    void selectedFileChanged(const QString& filename, bool isExample);
    void loadFile(const QString& filename, bool isExample);
    
private:
    bool updateModulesWorkspaces(TreeItem* titleItem, QLayout* container, std::vector<std::pair<QLabel*, FixedSizeListView*>>& workspaceViewsList);
    void updateWorkspaceViewVisibility();
    void listViewDoubleClicked(const QModelIndex& index);
    void setupView(QListView* view);
    QLabel* createRichTextLabel(std::string_view text) const;
    
    QLabel* noWorkspacesLabel_;
    QLabel* recentWorkspacesLabel_;
    QVBoxLayout* examples_;
    QLabel* examplesLabel_;
    QVBoxLayout* regressionTests_;
    QLabel* regressionTestsLabel_;
    FixedSizeListView* recentWorkspaces_;
    std::vector<std::pair<QLabel*, FixedSizeListView*>> examplesViewList_;
    std::vector<std::pair<QLabel*, FixedSizeListView*>> regressionTestViewList_;

    FileTreeModel* model_;
    QSortFilterProxyModel* proxyModel_;
    QItemSelectionModel* selectionModel_;

};

}  // namespace inviwo
