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
#include <modules/qtwidgets/inviwoqtutils.h>

#include <inviwo/qt/editor/workspacemodelroles.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QStyle>
#include <QApplication>
#include <QSortFilterProxyModel>
#include <QHeaderView>
#include <QPainterPath>
#include <QIdentityProxyModel>
#include <QHash>
#include <QResizeEvent>
#include <QPersistentModelIndex>
#include <QScrollBar>
#include <warn/pop>

namespace inviwo {

using Role = WorkspaceModelRole;
using Type = WorkspaceModelType;

namespace {

class SectionDelegate : public QStyledItemDelegate {
public:
    SectionDelegate(int itemSize, QWidget* parent = nullptr);
    virtual ~SectionDelegate() override = default;

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option,
                       const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    QImage rightArrow;
    QImage downArrow;
    int itemSize_;
};

SectionDelegate::SectionDelegate(int itemSize, QWidget* parent)
    : QStyledItemDelegate(parent)
    , rightArrow{":/svgicons/arrow-right-enabled.svg"}
    , downArrow{":/svgicons/arrow-down-enabled.svg"}
    , itemSize_(itemSize) {}

void SectionDelegate::paint(QPainter* painter, const QStyleOptionViewItem& o,
                            const QModelIndex& index) const {

    auto option = o;
    initStyleOption(&option, index);
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
                            QPainter::SmoothPixmapTransform);

    painter->save();
    if (utilqt::getData(index, Role::Type) == Type::File) {

        option.text = "";
        QStyle* style = option.widget ? option.widget->style() : QApplication::style();
        style->drawControl(QStyle::CE_ItemViewItem, &option, painter, option.widget);

        const auto name = utilqt::getData(index, Qt::DisplayRole).toString();
        const auto path = utilqt::getData(index, Role::Path).toString();
        const auto image = utilqt::getData(index, Role::PrimaryImage).value<QImage>();

        const auto margin = utilqt::emToPx(option.fontMetrics, 0.5);

        const auto baseRect = option.rect.adjusted(margin, margin, -margin, -margin);

        const auto txtHeight = baseRect.height() / 4;
        const auto imgCenter = baseRect.adjusted(0, 0, 0, -txtHeight - margin).center();
        const auto w = std::min(baseRect.width(), baseRect.height() - txtHeight - margin) / 2;
        const auto imgRect = QRect{imgCenter, QSize{0, 0}}.adjusted(-w, -w, w, w);

        QPainterPath border;
        border.addRoundedRect(imgRect, 35, 35, Qt::RelativeSize);

        const auto is = std::min(image.rect().width(), image.rect().height()) / 2;
        const auto sourceRect =
            QRect{image.rect().center(), QSize{0, 0}}.adjusted(-is, -is, is, is);
        painter->setClipPath(border, Qt::ReplaceClip);
        painter->drawImage(imgRect, image, sourceRect);
        painter->setClipPath(border, Qt::NoClip);

        painter->setPen(QPen{option.palette.text().color(), 1.5});
        painter->drawPath(border);

        const auto nameRect = baseRect.adjusted(0, baseRect.height() - txtHeight, 0, 0);
        painter->setPen(option.palette.text().color().lighter());
        painter->drawText(nameRect, Qt::AlignHCenter | Qt::AlignTop | Qt::TextWrapAnywhere, name);

    } else if (index.column() == 0) {
        // enlarge and emphasize font of section headers
        option.font.setBold(true);
        option.font.setPointSizeF(option.font.pointSizeF() * 1.2);

        option.text = "";
        QStyle* style = option.widget ? option.widget->style() : QApplication::style();
        style->drawControl(QStyle::CE_ItemViewItem, &option, painter, option.widget);

        painter->setClipping(false);

        const auto name = utilqt::getData(index, Qt::DisplayRole).toString();

        const auto& img = option.state & QStyle::State_Open ? downArrow : rightArrow;

        const auto indent = style->pixelMetric(QStyle::PM_TreeViewIndentation, 0, option.widget);

        const auto level = [&]() {
            auto i = index.parent();
            int l = 0;
            while (i.isValid()) {
                i = i.parent();
                ++l;
            }
            return l;
        }();

        auto rect = option.rect;
        const auto imgRect = QRect{
            QPoint{level * indent, rect.center().y() - img.rect().center().y()}, img.rect().size()};
        painter->drawImage(imgRect, img, img.rect());

        auto nameRect = option.rect;
        nameRect.adjust(level * indent + rect.height(), 0, 0, 0);

        painter->setFont(option.font);
        painter->drawText(nameRect,
                          Qt::AlignLeft | Qt::AlignVCenter | Qt::TextDontClip | Qt::TextSingleLine,
                          name);
    }
    painter->restore();
}

QSize SectionDelegate::sizeHint(const QStyleOptionViewItem& o, const QModelIndex& index) const {
    if (!index.isValid()) return QSize();

    auto size = QStyledItemDelegate::sizeHint(o, index);
    if (utilqt::getData(index, Role::Type) == Type::File) {
        size.setHeight(itemSize_);
    }
    return size;
}

}  // namespace

class ChunkProxyModel : public QAbstractProxyModel {
public:
    ChunkProxyModel(QAbstractItemModel* model, int chunkSize, QObject* parent)
        : QAbstractProxyModel(parent), chunkSize_{chunkSize} {

        setSourceModel(model);
        auto reset = [this]() {
            beginResetModel();
            sourceIndexMapping_.clear();
            endResetModel();
        };
        connect(model, &QAbstractItemModel::rowsAboutToBeInserted, this, reset);
        connect(model, &QAbstractItemModel::rowsAboutToBeRemoved, this, reset);
        connect(model, &QAbstractItemModel::rowsAboutToBeMoved, this, reset);
        connect(model, &QAbstractItemModel::columnsAboutToBeInserted, this, reset);
        connect(model, &QAbstractItemModel::columnsAboutToBeRemoved, this, reset);
        connect(model, &QAbstractItemModel::columnsAboutToBeMoved, this, reset);
        connect(model, &QAbstractItemModel::modelAboutToBeReset, this, reset);
        connect(model, &QAbstractItemModel::layoutAboutToBeChanged, this, reset);

        connect(
            model, &QAbstractItemModel::dataChanged, this,
            [this](const QModelIndex& topLeft, const QModelIndex& bottomRight, const auto& roles) {
                for (int i = topLeft.row(); i <= bottomRight.row(); ++i) {
                    auto changed = mapFromSource(sourceModel()->index(i, 0, topLeft.parent()));
                    dataChanged(changed, changed, roles);
                }
            });
    }

    virtual int columnCount(
        [[maybe_unused]] const QModelIndex& parent = QModelIndex()) const override {
        return chunkSize_;
    }
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        auto sourceParent = mapToSource(parent);
        auto sourceRows = sourceModel()->rowCount(sourceParent);

        if (utilqt::getData(sourceParent, Role::Type) == Type::SubSection) {
            auto chunkedRows = (sourceRows + chunkSize_ - 1) / chunkSize_;
            return chunkedRows;
        } else {
            return sourceRows;
        }
    }

    QModelIndex mapFromSource(const QModelIndex& sourceIndex) const override {
        if (!sourceIndex.isValid()) return {};

        auto map = mapping(sourceIndex);
        if (utilqt::getData(sourceIndex, Role::Type) == Type::File) {
            const auto row = sourceIndex.row() / chunkSize_;
            const auto col = sourceIndex.row() % chunkSize_;
            return createIndex(row, col, map);
        } else {
            return createIndex(sourceIndex.row(), sourceIndex.column(), map);
        }
    }

    QModelIndex mapToSource(const QModelIndex& proxyIndex) const override {
        if (!proxyIndex.isValid()) return {};

        auto m = static_cast<const Mapping*>(proxyIndex.internalPointer());
        return sourceModel()->index(m->sourceRow, m->sourceCol, m->sourceParent);
    }

    QModelIndex parent(const QModelIndex& child) const override {
        const QModelIndex sourceIndex = mapToSource(child);
        const QModelIndex sourceParent = sourceIndex.parent();
        return mapFromSource(sourceParent);
    }

    QModelIndex index(int row, int column, const QModelIndex& parent) const override {
        const QModelIndex sourceParent = mapToSource(parent);

        if (utilqt::getData(sourceParent, Role::Type) == Type::SubSection) {
            const int sourceRow = row * chunkSize_ + column;
            return mapFromSource(sourceModel()->index(sourceRow, 0, sourceParent));
        } else {
            return mapFromSource(sourceModel()->index(row, column, sourceParent));
        }
    }

    QModelIndex sibling(int row, int column, const QModelIndex& idx) const override {
        if (!idx.isValid()) return {};
        return index(row, column, idx.parent());
    }

    void setChunkSize(int chunkSize) {
        if (chunkSize_ == chunkSize) return;

        beginResetModel();
        chunkSize_ = chunkSize;
        sourceIndexMapping_.clear();
        endResetModel();
    }

    int chunkSize() const { return chunkSize_; }

private:
    struct Hash {
        size_t operator()(const QModelIndex& idx, size_t seed = std::hash<int>{}(0)) const {
            return qHash(idx, seed);
        }
    };
    struct Mapping {
        int sourceRow;
        int sourceCol;
        QPersistentModelIndex sourceParent;
    };
    using IndexMap = std::unordered_map<QModelIndex, Mapping, Hash>;

    Mapping* mapping(const QModelIndex& sourceIndex) const {
        auto [it, inserted] = sourceIndexMapping_.try_emplace(sourceIndex);
        if (inserted) {
            it->second.sourceRow = sourceIndex.row();
            it->second.sourceCol = sourceIndex.column();
            it->second.sourceParent = sourceIndex.parent();
        }
        return &(it->second);
    }
    mutable IndexMap sourceIndexMapping_;
    int chunkSize_;
};

WorkspaceGridView::WorkspaceGridView(QAbstractItemModel* theModel, QWidget* parent)
    : QTreeView{parent}
    , itemSize_{utilqt::emToPx(this, 14)}
    , proxy_{new ChunkProxyModel{theModel, 3, this}} {

    setModel(proxy_);

    setHeaderHidden(true);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    setItemDelegate(new SectionDelegate(itemSize_, this));
    setIndentation(0);

#if defined(WIN32)
    // Scrolling on Windows is set to scroll per item by default. Also need to adjust the step size
    // since the default appears to be based on the number of items in the view.
    setVerticalScrollMode(ScrollPerPixel);
    verticalScrollBar()->setSingleStep(utilqt::emToPx(parent, 1.5));
#endif

    connect(this, &QTreeView::doubleClicked, this, [this](const QModelIndex& index) {
        if (index.isValid() && (utilqt::getData(index, Role::Type) == Type::File)) {
            const auto filename = utilqt::getData(index, Role::FilePath).toString();
            const auto isExample = utilqt::getData(index, Role::isExample).toBool();
            emit loadFile(filename, isExample);
        }
    });

    connect(this, &QTreeView::clicked, this, [this](const QModelIndex& index) {
        if (index.isValid() && (utilqt::getData(index, Role::Type) != Type::File)) {

            if (QGuiApplication::keyboardModifiers() & Qt::ControlModifier) {
                isExpanded(index) ? collapseRecursively(index) : expandRecursively(index);
            } else {
                setExpanded(index, !isExpanded(index));
            }
        }
    });

    connect(selectionModel(), &QItemSelectionModel::currentChanged, this,
            [this](const QModelIndex& current, const QModelIndex&) {
                if (current.isValid() && (utilqt::getData(current, Role::Type) == Type::File)) {
                    emit selectFile(current);
                } else {
                    emit selectFile(QModelIndex{});
                }
            });

    header()->setSectionResizeMode(QHeaderView::Stretch);
}

const QAbstractProxyModel& WorkspaceGridView::proxy() const { return *proxy_; }

void WorkspaceGridView::resizeEvent(QResizeEvent* event) {
    QTreeView::resizeEvent(event);

    const int newChunkSize = event->size().width() / itemSize_;

    if (newChunkSize != proxy_->chunkSize()) {
        std::vector<QModelIndex> expanded;
        auto findExpaned = [&](auto& self, const QModelIndex& parent) -> void {
            auto rows = proxy_->rowCount(parent);
            for (int i = 0; i < rows; ++i) {
                auto index = proxy_->index(i, 0, parent);
                if (isExpanded(index)) {
                    expanded.push_back(proxy_->mapToSource(index));
                    self(self, index);
                }
            }
        };
        findExpaned(findExpaned, QModelIndex{});
        proxy_->setChunkSize(event->size().width() / itemSize_);
        for (const auto& idx : expanded) {
            expand(proxy_->mapFromSource(idx));
        }
    }
}

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
// QTreeView::expandRecursively() was introduced in Qt 5.13
// see https://doc.qt.io/qt-5/qtreeview.html#expandRecursively
void WorkspaceGridView::expandRecursively(const QModelIndex& index) {
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

void WorkspaceGridView::collapseRecursively(const QModelIndex& index) {
    if (index.isValid()) {
        for (int i = 0; i < model()->rowCount(index); ++i) {
            collapseRecursively(model()->index(i, 0, index));
        }
        if (isExpanded(index)) {
            collapse(index);
        }
    }
}

}  // namespace inviwo
