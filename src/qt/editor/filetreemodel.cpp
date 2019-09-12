/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <inviwo/qt/editor/filetreemodel.h>

#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stdextensions.h>
#include <modules/qtwidgets/inviwoqtutils.h>

namespace inviwo {

bool operator==(const QVariant& v, FileTreeModel::ListElemType t) {
    return v.toInt() == static_cast<int>(t);
}

bool operator==(FileTreeModel::ListElemType t, const QVariant& v) { return operator==(v, t); }

bool operator!=(const QVariant& v, FileTreeModel::ListElemType t) {
    return v.toInt() != static_cast<int>(t);
}

bool operator!=(FileTreeModel::ListElemType t, const QVariant& v) { return operator!=(v, t); }

FileTreeModel::FileTreeModel(QObject* parent)
    : QAbstractItemModel(parent), root_{std::make_unique<TreeItem>(nullptr)} {}

QModelIndex FileTreeModel::index(int row, int column, const QModelIndex& parent) const {
    if (parent.isValid() && parent.column() != 0) return QModelIndex();

    TreeItem* parentItem = getItem(parent);
    TreeItem* childItem = parentItem->child(row);

    if (childItem) return createIndex(row, column, childItem);
    return QModelIndex();
}

Qt::ItemFlags FileTreeModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index);
}

QVariant FileTreeModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return QVariant();
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());

    return item->data(index.column(), role);
}

QVariant FileTreeModel::headerData(int, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) return "Workspaces";

    return QVariant();
}

QModelIndex FileTreeModel::parent(const QModelIndex& index) const {
    if (!index.isValid()) return QModelIndex();

    TreeItem* childItem = getItem(index);
    TreeItem* parentItem = childItem ? childItem->parent() : nullptr;

    if (parentItem == root_.get() || !parentItem) return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int FileTreeModel::rowCount(const QModelIndex& parent) const {
    const TreeItem* parentItem = getItem(parent);
    return parentItem ? parentItem->childCount() : 0;
}

int FileTreeModel::columnCount(const QModelIndex&) const { return root_->columnCount(); }

bool FileTreeModel::insertRows(int position, int rows, const QModelIndex& parent) {
    TreeItem* parentItem = getItem(parent);
    if (!parentItem) return false;
    if ((position < 0) || (position >= parentItem->childCount())) return false;

    beginInsertRows(parent, position, position + rows - 1);
    const bool success = parentItem->insertChildren(position, rows);
    endInsertRows();

    return success;
}

bool FileTreeModel::removeRows(int position, int rows, const QModelIndex& parent) {
    TreeItem* parentItem = getItem(parent);
    if (!parentItem) return false;
    if ((position < 0) || (position + rows > parentItem->childCount())) return false;

    beginRemoveRows(parent, position, position + rows - 1);
    const bool success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

void FileTreeModel::updateCategory(TreeItem* item,
                                   std::vector<std::unique_ptr<TreeItem>> children) {
    if (!item) return;

    // if children are identical, skip update
    if (children.size() == static_cast<size_t>(item->childCount())) {
        bool match = true;
        for (int i = 0; i < item->childCount() && match; ++i) {
            match &= children[i].get()->operator==(*item->child(i));
        }
        if (match) return;
    }

    QModelIndex index = getIndex(item);

    if (item->childCount() > 0) {
        beginRemoveRows(index, 0, item->childCount() - 1);
        item->removeChildren();
        endRemoveRows();
    }
    if (!children.empty()) {
        beginInsertRows(index, 0, static_cast<int>(children.size() - 1));
        item->addChildren(std::move(children));
        endInsertRows();
    }
}

void FileTreeModel::addEntry(TreeItem* node, std::unique_ptr<TreeItem> child) {
    QModelIndex index = getIndex(node);
    int pos = node ? node->childCount() : rowCount();
    beginInsertRows(index, pos, pos);
    if (node) {
        node->addChild(std::move(child));
    } else {
        root_->addChild(std::move(child));
    }
    endInsertRows();
}

bool FileTreeModel::removeEntry(TreeItem* node) {
    return removeRows(node->row(), 1, getIndex(node->parent()));
}

bool FileTreeModel::removeChildren(TreeItem* root) {
    if (!root || root == root_.get()) {
        beginResetModel();
        root_->removeChildren();
        endResetModel();
        return true;
    } else {
        return removeRows(0, root->childCount(), getIndex(root));
    }
}

QModelIndex FileTreeModel::getIndex(TreeItem* item, int column) const {
    if (!item) return QModelIndex();

    return createIndex(item->row(), column, item);
}

TreeItem* FileTreeModel::getItem(const QModelIndex& index) const {
    if (index.isValid()) {
        TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
        if (item) return item;
    }
    return root_.get();
}

TreeItem::TreeItem(TreeItem* parent) : parent_{parent} {}

TreeItem::TreeItem(const QString& caption, FileTreeModel::ListElemType type, TreeItem* parent)
    : parent_{parent}, type_{type}, caption_{caption} {}

TreeItem::TreeItem(const QIcon& icon, const std::string& filename, bool isExample, TreeItem* parent)
    : parent_{parent} {
    setData(icon, filename, isExample);
}

void TreeItem::addChild(std::unique_ptr<TreeItem> child) {
    child->parent_ = this;
    childItems_.push_back(std::move(child));
}

void TreeItem::addChildren(std::vector<std::unique_ptr<TreeItem>> children) {
    for (auto& child : children) {
        child->parent_ = this;
    }
    std::move(children.begin(), children.end(), std::back_inserter(childItems_));
}

bool TreeItem::insertChildren(int position, int count) {
    if (position < 0 || position > static_cast<int>(childItems_.size())) return false;

    for (int row = 0; row < count; ++row) {
        auto item = std::make_unique<TreeItem>(this);
        childItems_.insert(childItems_.begin() + position, std::move(item));
    }

    return true;
}

bool TreeItem::removeChildren(int position, int count) {
    if (position < 0 || position + count > static_cast<int>(childItems_.size())) return false;

    childItems_.erase(childItems_.begin() + position);

    return true;
}

void TreeItem::removeChildren() { childItems_.clear(); }

TreeItem* TreeItem::child(int row) const {
    if ((row < 0) || (row >= static_cast<int>(childItems_.size()))) return nullptr;
    return childItems_[row].get();
}

int TreeItem::row() const {
    if (parent_) {
        auto it = util::find_if(
            parent_->childItems_,
            [node = const_cast<TreeItem*>(this)](auto& child) { return child.get() == node; });
        if (it != parent_->childItems_.end()) {
            return static_cast<int>(std::distance(parent_->childItems_.begin(), it));
        }
    }
    return 0;
}

int TreeItem::childCount() const { return static_cast<int>(childItems_.size()); }

int TreeItem::columnCount() const { return 2; }

TreeItem* TreeItem::parent() const { return parent_; }

QVariant TreeItem::data(int column, int role) const {
    if ((column < 0) || (column >= columnCount())) {
        return {};
    }

    if (type_ == FileTreeModel::ListElemType::File) {
        switch (role) {
            case Qt::DisplayRole:
                return caption_;
            case Qt::EditRole:
                return caption_;
            case Qt::ToolTipRole: {
                QStringList list;
                auto p = parent();
                while (p) {
                    list.append(p->caption_);
                    p = p->parent();
                }
                std::reverse(list.begin(), list.end());

                const QString hierarchy =
                    (list.size() > 0) ? QString("(%1)").arg(list.join("/").remove(0, 1)) : "";

                // prevent line breaks in the tooltip
                // see https://doc.qt.io/qt-5/qtooltip.html#details
                return QString("<p style='white-space:pre'><strong>%1</strong> %3<br>%2</p>")
                    .arg(file_)
                    .arg(path_)
                    .arg(hierarchy);
            }
            case Qt::DecorationRole:
                if (column == 0) {
                    return {};
                } else {
                    return icon_;
                }
            case FileTreeModel::ItemRoles::Type:
                return static_cast<int>(type_);
            case FileTreeModel::ItemRoles::FileName:
                return file_;
            case FileTreeModel::ItemRoles::Path:
                return path_;
            case FileTreeModel::ItemRoles::ExampleWorkspace:
                return isExample_;
            default:
                return {};
        }
    } else {
        switch (role) {
            case Qt::DisplayRole:
            case Qt::EditRole:
            case Qt::ToolTipRole:
                return caption_;
            case FileTreeModel::ItemRoles::Type:
                return static_cast<int>(type_);
            default:
                return {};
        }
    }
}

FileTreeModel::ListElemType TreeItem::type() const { return type_; }

void TreeItem::setData(const QString& caption, FileTreeModel::ListElemType type) {
    type_ = type;
    caption_ = caption;
}

void TreeItem::setData(const QIcon& icon, const std::string& filename, bool isExample) {
    type_ = FileTreeModel::ListElemType::File;

    icon_ = icon;
    caption_ = utilqt::toQString(filename);
    file_ = utilqt::toQString(filesystem::getFileNameWithExtension(filename));
    path_ = utilqt::toQString(filesystem::getFileDirectory(filename));
    isExample_ = isExample;

    if (path_.isEmpty()) {
        path_ = ".";
    }
}

bool TreeItem::operator==(const TreeItem& tree) const {
    bool match = (type_ == tree.type_) && (caption_ == tree.caption_) &&
                 (isExample_ == tree.isExample_) && (childCount() == tree.childCount());
    // check children
    if (match) {
        for (int i = 0; i < childCount() && match; ++i) {
            match &= childItems_[i].get()->operator==(*tree.child(i));
        }
    }

    return match;
}

bool TreeItem::operator!=(const TreeItem& tree) const { return !operator==(tree); }

}  // namespace inviwo
