/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2021 Inviwo Foundation
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

#include <inviwo/qt/editor/workspacetreeview.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/network/workspaceannotations.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stdextensions.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <tuple>

#include <warn/push>
#include <warn/ignore/all>

#include <QStyledItemDelegate>
#include <QPainter>
#include <QStyle>
#include <QApplication>
#include <QSortFilterProxyModel>
#include <QHeaderView>

#include <warn/pop>

namespace inviwo {

namespace {

class SectionDelegate : public QStyledItemDelegate {
public:
    SectionDelegate(QWidget* parent = nullptr);
    virtual ~SectionDelegate() override = default;

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option,
                       const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    std::tuple<QRect, QRect, QRect> getTextBoundingBox(const QStyleOptionViewItem& option,
                                                       const QModelIndex& index) const;

    static QString elidedText(const QString& str, const QFontMetrics& metrics, int width);
};

SectionDelegate::SectionDelegate(QWidget* parent) : QStyledItemDelegate(parent) {}

void SectionDelegate::paint(QPainter* painter, const QStyleOptionViewItem& o,
                            const QModelIndex& index) const {

    if (index.data(WorkspaceTreeModel::ItemRoles::Type).toInt() ==
        WorkspaceTreeModel::ListElemType::File) {
        auto option = o;
        initStyleOption(&option, index);

        option.text = "";
        QStyle* style = option.widget ? option.widget->style() : QApplication::style();
        style->drawControl(QStyle::CE_ItemViewItem, &option, painter, option.widget);

        if (index.column() > 0) {
            const auto filename = index.data(WorkspaceTreeModel::ItemRoles::FileName).toString();
            const auto path = index.data(WorkspaceTreeModel::ItemRoles::Path).toString();
            auto boundingRects = getTextBoundingBox(option, index);

            // draw text
            painter->save();
            auto fontFilename = option.font;
            fontFilename.setBold(true);
            painter->setFont(fontFilename);
            painter->setPen(option.palette.text().color().lighter());
            painter->drawText(std::get<1>(boundingRects),
                              Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, filename);

            painter->setFont(option.font);
            painter->setPen(option.palette.text().color());
            painter->drawText(
                std::get<2>(boundingRects), Qt::AlignLeft | Qt::AlignTop,
                elidedText(path, option.fontMetrics, std::get<0>(boundingRects).width()));
            painter->restore();
        }
    } else {
        auto option = o;
        initStyleOption(&option, index);
        // enlarge and emphasize font of section headers
        option.font.setBold(true);
        option.font.setPointSizeF(option.font.pointSizeF() * 1.2);

        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize SectionDelegate::sizeHint(const QStyleOptionViewItem& o, const QModelIndex& index) const {
    if (!index.isValid()) return QSize();

    if (index.data(WorkspaceTreeModel::ItemRoles::Type).toInt() ==
        WorkspaceTreeModel::ListElemType::File) {
        auto option = o;
        initStyleOption(&option, index);

        auto boundingRects = getTextBoundingBox(option, index);

        auto sizehint = QSize{option.rect.width(), std::get<0>(boundingRects).height()};

        if (sizehint.height() < option.decorationSize.height()) {
            sizehint.setHeight(option.decorationSize.height());
        }
        return sizehint;
    } else {
        return QStyledItemDelegate::sizeHint(o, index);
    }
}

std::tuple<QRect, QRect, QRect> SectionDelegate::getTextBoundingBox(
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
    if (!index.isValid()) return {};

    const auto filename = index.data(WorkspaceTreeModel::ItemRoles::FileName).toString();
    const auto path = index.data(WorkspaceTreeModel::ItemRoles::Path).toString();

    auto fontFilename = option.font;
    fontFilename.setBold(true);
    const auto fm = QFontMetrics(fontFilename);

    const int marginLeft = utilqt::emToPx(fm, 6.0 / utilqt::refEm());
    const int textSpacing = utilqt::emToPx(fm, 4.0 / utilqt::refEm());
    const int margin = utilqt::emToPx(fm, 5.0 / utilqt::refEm());
    const int marginRight = utilqt::emToPx(fm, 6.0 / utilqt::refEm());

    auto textRect = (option.rect.isValid() ? option.rect : QRect());
    textRect.adjust(marginLeft + option.decorationSize.width(), margin, -marginRight, 0);
    // set rect height to zero, since the font metric will calculate the required height of
    // the text
    textRect.setHeight(0);

    auto filenameRect = fm.boundingRect(textRect, Qt::AlignLeft | Qt::AlignTop, filename);

    textRect.setTop(filenameRect.bottom() + textSpacing);
    auto pathRect = option.fontMetrics.boundingRect(textRect, Qt::AlignLeft | Qt::AlignTop, path);

    auto boundingRect = filenameRect.united(pathRect);
    boundingRect.adjust(0, -margin, 0, margin);
    boundingRect.setRight(option.rect.right() - marginRight);

    return {boundingRect, filenameRect, pathRect};
}

QString SectionDelegate::elidedText(const QString& str, const QFontMetrics& metrics, int width) {
    if (str.isEmpty() || (metrics.boundingRect(str).width() <= width)) {
        return str;
    }

    auto directories = str.split('/');

    bool keepFirst = str[0] != '/';
    const int widthFirst = (keepFirst ? metrics.boundingRect(directories.front()).width() : 0);
    const QString strFirst = (keepFirst ? directories.front() : "");
    if (keepFirst) {
        directories.erase(directories.begin());
    }

    std::reverse(directories.begin(), directories.end());

    std::vector<int> widthDirs;
    std::transform(directories.begin(), directories.end(), std::back_inserter(widthDirs),
                   [&](const auto& dir) { return metrics.boundingRect("/" + dir).width(); });

    const int widthDots = metrics.boundingRect("/...").width();

    if (widthFirst + widthDots + widthDirs.front() > width) {
        // eliding path components is not sufficient, elide entire string
        return metrics.elidedText(str, Qt::ElideRight, width);
    }

    int leftWidth = width - widthFirst - widthDots;
    QString result =
        std::accumulate(directories.begin(), directories.end(), QString(),
                        [&, index = 0, leftWidth](QString str, const QString& dir) mutable {
                            if (leftWidth >= widthDirs[index]) {
                                str.prepend("/" + dir);
                                leftWidth -= widthDirs[index];
                            } else {
                                leftWidth = 0;
                            }
                            ++index;
                            return str;
                        });
    return strFirst + "/..." + result;
}

}  // namespace

WorkspaceTreeView::WorkspaceTreeView(WorkspaceTreeModel* model,
                                     QSortFilterProxyModel* workspaceProxyModel,
                                     QItemSelectionModel* workspaceSelectionModel, QWidget* parent)
    : QTreeView{parent}, model_{model}, proxyModel_{workspaceProxyModel} {

    setModel(proxyModel_);
    setSelectionModel(workspaceSelectionModel);

    setHeaderHidden(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setIconSize(utilqt::emToPx(this, QSizeF(2.5, 2.5)));
    setIndentation(utilqt::emToPx(this, 1.0));
    setItemDelegate(new SectionDelegate(this));

    // adjust width of first column
    // file entries and icons start in column 2, sections headers span all columns
    header()->setMinimumSectionSize(0);
    header()->resizeSection(0, utilqt::emToPx(this, 2.0));
    QObject::connect(model_, &WorkspaceTreeModel::recentWorkspacesUpdated, this,
                     [this](TreeItem* recentWorkspaceItem) {
                         if (!recentWorkspaceItem_) {
                             recentWorkspaceItem_ = recentWorkspaceItem;
                             auto index =
                                 proxyModel_->mapFromSource(model_->getIndex(recentWorkspaceItem));
                             expand(index);
                             setFirstColumnSpanned(index.row(), index.parent(), true);
                         }
                     });
    QObject::connect(model_, &WorkspaceTreeModel::exampleWorkspacesUpdated, this,
                     [this](TreeItem* exampleWorkspaceItem) {
                         updateWorkspaces(examplesItem_, exampleWorkspaceItem);
                     });
    QObject::connect(model_, &WorkspaceTreeModel::regressionTestWorkspacesUpdated, this,
                     [this](TreeItem* regressionTestWorkspaceItem) {
                         updateWorkspaces(regressionTestsItem_, regressionTestWorkspaceItem);
                     });

    QObject::connect(
        selectionModel(), &QItemSelectionModel::currentRowChanged, this,
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

    QObject::connect(this, &QTreeView::doubleClicked, this, [this](const QModelIndex& index) {
        if (index.isValid() && (index.data(WorkspaceTreeModel::ItemRoles::Type) ==
                                WorkspaceTreeModel::ListElemType::File)) {
            const auto filename = index.data(WorkspaceTreeModel::ItemRoles::Path).toString() + "/" +
                                  index.data(WorkspaceTreeModel::ItemRoles::FileName).toString();
            const auto isExample =
                index.data(WorkspaceTreeModel::ItemRoles::ExampleWorkspace).toBool();
            emit loadFile(filename, isExample);
        }
    });
}
bool WorkspaceTreeView::selectRecentWorkspace(int index) {
    if (!recentWorkspaceItem_) return false;
    if (recentWorkspaceItem_->childCount() < index) return false;

    auto idx = proxyModel_->mapFromSource(model_->getIndex(recentWorkspaceItem_->child(index)));
    selectionModel()->setCurrentIndex(
        idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

    return true;
}

void WorkspaceTreeView::expandItems() {
    // fold/unfold all tree items based on filtering
    if (proxyModel_->filterRegularExpression().pattern().isEmpty()) {
        defaultExpand();
    } else {
        expandAll();
    }
}

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
//
// QTreeView::expandRecursively() was introduced in Qt 5.13
// see https://doc.qt.io/qt-5/qtreeview.html#expandRecursively
//
void WorkspaceTreeView::expandRecursively(const QModelIndex& index) {
    if (index.isValid()) {
        for (int i = 0; i < index.model()->rowCount(index); ++i) {
            expandRecursively(index.child(i, 0));
        }
        if (!isExpanded(index)) {
            expand(index);
        }
    }
}
#endif

void WorkspaceTreeView::defaultExpand() {
    setUpdatesEnabled(false);

    expandRecursively(proxyModel_->mapFromSource(model_->getIndex(recentWorkspaceItem_)));
    expandRecursively(proxyModel_->mapFromSource(model_->getIndex(examplesItem_)));

    auto index = proxyModel_->mapFromSource(model_->getIndex(regressionTestsItem_));
    expandRecursively(index);
    collapse(index);

    setUpdatesEnabled(true);
}

void WorkspaceTreeView::updateWorkspaces(TreeItem* currentWorkspaceItem,
                                         TreeItem* newWorkspaceItem) {

    if (!currentWorkspaceItem) {
        currentWorkspaceItem = newWorkspaceItem;
        auto index = proxyModel_->mapFromSource(model_->getIndex(newWorkspaceItem));
        collapse(index);
        setFirstColumnSpanned(index.row(), index.parent(), true);
    }
    bool isEmpty = newWorkspaceItem->childCount() == 0;
    setRowHidden(newWorkspaceItem->row(), QModelIndex(), isEmpty);

    if (!isEmpty) {
        setUpdatesEnabled(false);
        for (int i = 0; i < newWorkspaceItem->childCount(); ++i) {
            TreeItem* item = newWorkspaceItem->child(i);
            auto index = proxyModel_->mapFromSource(model_->getIndex(item));
            expand(index);
            setFirstColumnSpanned(index.row(), index.parent(), true);
        }
        setUpdatesEnabled(true);
    }
}

}  // namespace inviwo
