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

#include <inviwo/qt/editor/workspacegridview.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/network/workspaceannotations.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/zip.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <tuple>

#include <warn/push>
#include <warn/ignore/all>

#include <QStyledItemDelegate>
#include <QPainter>
#include <QStyle>
#include <QApplication>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>

#include <warn/pop>

namespace inviwo {

void FixedSizeListView::setModel(QAbstractItemModel* model) {
    
    if (model_ != nullptr) {
        model_->disconnect(this);
    }
    if (auto sfModel = dynamic_cast<QSortFilterProxyModel*>(model)) {
        QObject::connect(sfModel, &QAbstractItemModel::layoutChanged, this, &FixedSizeListView::checkRootIndex);
        QObject::connect(sfModel, &QAbstractItemModel::rowsRemoved, this, &FixedSizeListView::checkRootIndex);
        QObject::connect(sfModel, &QAbstractItemModel::rowsInserted, this, &FixedSizeListView::checkRootIndex);
        model_ = sfModel;
    } else {
        model_ = nullptr;
    }
    QListView::setModel(model);
}
QSize FixedSizeListView::sizeHint() const
{
    QSize hint = QListView::sizeHint();
    if (model()->rowCount() > 0) {
        auto width = contentsRect().width();
        auto nItems = model()->rowCount(rootIndex());
        auto itemHeight = sizeHintForRow(0);
        auto nItemsPerRow = floor(width / std::max(1, itemHeight));
        auto nRows = std::max(1, static_cast<int>(std::ceil(nItems / nItemsPerRow)));
        hint.setWidth(width);
        hint.setHeight(nRows * itemHeight);
        return hint;
    }
    return hint;
}
void FixedSizeListView::checkRootIndex() {
    if (rootIndex().isValid() ||
        !model_) { return; }

    auto rootIndex = model_->mapFromSource(this->sourceRootIndex_);
    if (rootIndex != this->rootIndex()) {
        // Prevent segmentation faults
        // See
        // https://stackoverflow.com/questions/70112321/qt-rootindex-gets-reset-each-time-qsortfilterproxymodelinvalidatefilter-is-c
        rootIndex = rootIndex.model()->index(
                       rootIndex.row(),
                       rootIndex.column(),
                                            rootIndex.parent());
        QListView::setRootIndex(rootIndex);
    }
}

void FixedSizeListView::setRootIndex(const QModelIndex& rootIndex) {
    QListView::setRootIndex(rootIndex);
    if (model_) {
        auto mappedRootIndex = model_->mapToSource(rootIndex);
        this->sourceRootIndex_ = QPersistentModelIndex(mappedRootIndex);
    }

}


WorkspaceGridView::WorkspaceGridView(WorkspaceTreeModel* model, QSortFilterProxyModel* workspaceProxyModel,
                                     QItemSelectionModel* selectionModel, QWidget* parent)
    : QWidget{parent}
    , model_(model)
    , proxyModel_(workspaceProxyModel)
    , selectionModel_(selectionModel) {
    
    noWorkspacesLabel_ = createRichTextLabel("<h2>Could not find any workspaces..</h2>");
    noWorkspacesLabel_->setVisible(false);
        
    recentWorkspaces_ = new FixedSizeListView();
    setupView(recentWorkspaces_);
    
    recentWorkspacesLabel_ = createRichTextLabel("<h2>Recent workspaces</h2>");
    examples_ = new QVBoxLayout();
    //examples_->setSizeConstraint(QLayout::SetFixedSize);
    examplesLabel_ = createRichTextLabel("<h2>Example workspaces</h2>");
    regressionTests_ = new QVBoxLayout();
    //regressionTests_->setSizeConstraint(QLayout::SetFixedSize);
    regressionTestsLabel_ = createRichTextLabel("<h2>Regression test workspaces</h2>");
        
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(utilqt::emToPx(this, 1));
    layout->addWidget(noWorkspacesLabel_);
    layout->addWidget(recentWorkspacesLabel_);
    layout->addWidget(recentWorkspaces_);
    layout->addWidget(examplesLabel_);
    layout->addLayout(examples_);
    layout->addWidget(regressionTestsLabel_);
    layout->addLayout(regressionTests_);
        
    // React to filtering changes
    QObject::connect(workspaceProxyModel, &QSortFilterProxyModel::rowsRemoved, this,
                     &WorkspaceGridView::updateWorkspaceViewVisibility);
    QObject::connect(workspaceProxyModel, &QSortFilterProxyModel::rowsInserted, this,
                     &WorkspaceGridView::updateWorkspaceViewVisibility);
        
    QObject::connect(model_, &WorkspaceTreeModel::recentWorkspacesUpdated, this,
                     [this](TreeItem* recentWorkspaceItem) {
        auto index = proxyModel_->mapFromSource(model_->getIndex(recentWorkspaceItem));
        recentWorkspaces_->setRootIndex(index);
        bool isEmpty = !proxyModel_->hasChildren(index);
        recentWorkspaces_->setVisible(!isEmpty);
        recentWorkspacesLabel_->setVisible(!isEmpty);
    });
    QObject::connect(model_, &WorkspaceTreeModel::exampleWorkspacesUpdated, this,
                     [this](TreeItem* exampleWorkspaceItem) {
        examplesLabel_->setVisible(updateModulesWorkspaces(exampleWorkspaceItem, examples_, examplesViewList_));
     });
    QObject::connect(model_, &WorkspaceTreeModel::regressionTestWorkspacesUpdated, this,
                     [this](TreeItem* regressionTestWorkspaceItem) {
        regressionTestsLabel_->setVisible(updateModulesWorkspaces(regressionTestWorkspaceItem, regressionTests_, regressionTestViewList_));
     });
    
    QObject::connect(selectionModel_, &QItemSelectionModel::currentRowChanged, this,
                     [this](const QModelIndex& current, const QModelIndex&) {
                         if (current.isValid() && (current.data(WorkspaceTreeModel::ItemRoles::Type) ==
                                                   WorkspaceTreeModel::ListElemType::File)) {
                             const auto filename =
                                 current.data(WorkspaceTreeModel::ItemRoles::Path).toString() + "/" +
                                 current.data(WorkspaceTreeModel::ItemRoles::FileName).toString();
                             const auto isExample =
                                 current.data(WorkspaceTreeModel::ItemRoles::ExampleWorkspace).toBool();
                             emit selectedFileChanged(filename, isExample);
                         } else {
                             emit selectedFileChanged("", false);
                         }
                     });
        
}

bool WorkspaceGridView::updateModulesWorkspaces(TreeItem* titleItem, QLayout* container, std::vector<std::pair<QLabel*, FixedSizeListView*>>& workspaceViewsList) {
    while (QLayoutItem* child = container->takeAt(0)) {
        delete child->widget(); // delete the widget
        delete child;   // delete the layout item
    }
    workspaceViewsList.clear();
    
    for (int i = 0; i < titleItem->childCount(); ++i) {
        TreeItem* item = titleItem->child(i);
        auto index = proxyModel_->mapFromSource(model_->getIndex(item));
        auto title = utilqt::fromLocalQString(item->data(0, Qt::DisplayRole).toString());
        QLabel* titleLabel = createRichTextLabel(fmt::format("<h3>{}</h3>", title));
        container->addWidget(titleLabel);
        auto view = new FixedSizeListView();
        setupView(view);
        view->setRootIndex(index);
        workspaceViewsList.push_back(std::make_pair(titleLabel, view));
        container->addWidget(view);
        
    }
    return titleItem->childCount() != 0;
}
void WorkspaceGridView::updateWorkspaceViewVisibility() {
    bool recenWorkSpacesVisible = recentWorkspaces_->rootIndex().isValid();
    recentWorkspaces_->setVisible(recenWorkSpacesVisible);
    recentWorkspacesLabel_->setVisible(recenWorkSpacesVisible);

    bool anyExampleVisible = false;
    for (auto& labelView: examplesViewList_) {
       bool visible = labelView.second->rootIndex().isValid();
       labelView.first->setVisible(visible);
       labelView.second->setVisible(visible);
       anyExampleVisible |= visible;
    }
    examplesLabel_->setVisible(anyExampleVisible);
    bool anyRegressionTestVisible = false;
    for (auto& labelView: regressionTestViewList_) {
       bool visible = labelView.second->rootIndex().isValid();
       labelView.first->setVisible(visible);
       labelView.second->setVisible(visible);
       anyRegressionTestVisible |= visible;
    }
    regressionTestsLabel_->setVisible(anyRegressionTestVisible);
    noWorkspacesLabel_->setVisible(!(recenWorkSpacesVisible | anyExampleVisible | anyRegressionTestVisible));
}

void WorkspaceGridView::listViewDoubleClicked(const QModelIndex& index) {
    if (index.isValid() &&
        (index.data(WorkspaceTreeModel::ItemRoles::Type) == WorkspaceTreeModel::ListElemType::File)) {
        const auto filename = index.data(WorkspaceTreeModel::ItemRoles::Path).toString() + "/" +
                              index.data(WorkspaceTreeModel::ItemRoles::FileName).toString();
        const auto isExample = index.data(WorkspaceTreeModel::ItemRoles::ExampleWorkspace).toBool();
        emit loadFile(filename, isExample);
    }
}

void WorkspaceGridView::setupView(QListView* view) {
    view->setViewMode(QListView::ViewMode::IconMode);
    //view->setMinimumHeight(128);
    view->setIconSize(utilqt::emToPx(this, QSizeF(8, 8)));
    view->setResizeMode(QListView::ResizeMode::Adjust);
    view->setUniformItemSizes(true);
    view->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    view->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setMovement(QListView::Movement::Static);
    //view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setTextElideMode(Qt::TextElideMode::ElideMiddle);
    view->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
    view->setModel(proxyModel_);
    view->setSelectionModel(selectionModel_);
    QObject::connect(view, &QListView::doubleClicked, this, &WorkspaceGridView::listViewDoubleClicked);
}

QLabel* WorkspaceGridView::createRichTextLabel(std::string_view text) const {
    auto l = new QLabel(QString::fromStdString(std::string(text.data())));
    l->setTextFormat(Qt::RichText);
    return l;
};


}  // namespace inviwo
