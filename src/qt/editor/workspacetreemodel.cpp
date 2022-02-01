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

#include <inviwo/qt/editor/workspacetreemodel.h>

#include <inviwo/core/network/workspaceannotations.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/util/zip.h>
#include <inviwo/core/common/inviwoapplication.h>

#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/qt/editor/workspaceannotationsqt.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QFontMetrics>
#include <warn/pop>

namespace inviwo {

class TreeItem : public QObject {
public:
    static const int IconSize = 12;  // Icon size in em.

    explicit TreeItem(TreeItem* parent);
    TreeItem(std::string_view filename, WorkspaceTreeModel::Type type);
    TreeItem(std::string_view filename, InviwoApplication* app,
             std::function<void(TreeItem*)> onChange, bool isExample);

    TreeItem(const TreeItem&) = delete;
    TreeItem& operator=(const TreeItem&) = delete;

    void addChild(std::unique_ptr<TreeItem> child);
    void addChildren(std::vector<std::unique_ptr<TreeItem>> children);
    void removeChildren();

    TreeItem* child(int row) const;
    int row() const;

    int childCount() const;
    int columnCount() const;
    TreeItem* parent() const;

    QVariant data(int column, int role) const;
    Qt::ItemFlags flags() const;
    WorkspaceTreeModel::Type type() const;

    bool operator==(const TreeItem& tree) const;
    bool operator!=(const TreeItem& tree) const;

    TreeItem* findChild(std::string_view caption) const;

private:
    void initIconLoad() const;
    WorkspaceTreeModel::Type type_;
    TreeItem* parent_;
    int row_;
    std::vector<std::unique_ptr<TreeItem>> children_;

    QString caption_;
    QString file_;
    QString path_;
    bool isExample_ = false;

    std::shared_ptr<WorkspaceInfoLoader> infoLoader_;
    WorkspaceInfo info_;
};

void WorkspaceInfoLoader::operator()() {
    try {
        WorkspaceAnnotationsQt annotations{filename_, app_};
        emit workspaceInfoLoaded(WorkspaceInfo{
            utilqt::toQString(annotations.getTitle()), utilqt::toQString(annotations.getAuthor()),
            utilqt::toQString(annotations.getTags()),
            utilqt::toQString(annotations.getCategories()),
            utilqt::toQString(annotations.getDescription()),
            annotations.numberOfCanvases() > 0 ? annotations.getCanvasQImage(0) : QImage{},
            WorkspaceAnnotationsQt::workspaceProcessors(filename_, app_)});
    } catch (const Exception&) {
    }
}

void WorkspaceInfoLoader::submit() {
    std::call_once(flag_, [&]() { app_->dispatchPool([l = shared_from_this()]() { (*l)(); }); });
}

TreeItem::TreeItem(TreeItem* parent)
    : type_{WorkspaceTreeModel::Type::None}, parent_{parent}, row_{0} {}

TreeItem::TreeItem(std::string_view caption, WorkspaceTreeModel::Type type)
    : type_{type}, parent_{nullptr}, row_{0}, caption_{utilqt::toQString(caption)} {}

TreeItem::TreeItem(std::string_view filename, InviwoApplication* app,
                   std::function<void(TreeItem*)> onChange, bool isExample)
    : type_{WorkspaceTreeModel::Type::File}
    , parent_{nullptr}
    , row_{0}
    , caption_{utilqt::toQString(filesystem::getFileNameWithoutExtension(filename))}
    , file_{utilqt::toQString(filesystem::getFileNameWithExtension(filename))}
    , path_{utilqt::toQString(filesystem::getFileDirectory(filename))}
    , isExample_{isExample}
    , infoLoader_{std::make_shared<WorkspaceInfoLoader>(filename, app)}
    , info_{} {

    connect(
        infoLoader_.get(), &WorkspaceInfoLoader::workspaceInfoLoaded, this,
        [this, onChange](WorkspaceInfo info) {
            info_ = info;
            onChange(this);
        },
        Qt::QueuedConnection);

    if (path_.isEmpty()) path_ = ".";
}

void TreeItem::addChild(std::unique_ptr<TreeItem> child) {
    child->parent_ = this;
    child->row_ = static_cast<int>(children_.size());
    children_.push_back(std::move(child));
}

void TreeItem::addChildren(std::vector<std::unique_ptr<TreeItem>> children) {
    for (auto&& [i, child] : util::enumerate(children)) {
        child->parent_ = this;
        child->row_ = static_cast<int>(i + children_.size());
    }
    std::move(children.begin(), children.end(), std::back_inserter(children_));
}

void TreeItem::removeChildren() { children_.clear(); }

TreeItem* TreeItem::child(int row) const {
    if ((row < 0) || (row >= static_cast<int>(children_.size()))) return nullptr;
    return children_[row].get();
}

TreeItem* TreeItem::findChild(std::string_view caption) const {
    auto qCaption = utilqt::toQString(caption);
    auto it = std::find_if(children_.begin(), children_.end(),
                           [&](auto& child) { return child->caption_ == qCaption; });
    if (it != children_.end()) {
        return it->get();
    } else {
        return nullptr;
    }
}

int TreeItem::row() const { return row_; }

int TreeItem::childCount() const { return static_cast<int>(children_.size()); }

int TreeItem::columnCount() const { return 1; }

TreeItem* TreeItem::parent() const { return parent_; }

QVariant TreeItem::data(int column, int role) const {
    if ((column < 0) || (column >= columnCount())) {
        return {};
    }
    using Role = WorkspaceTreeModel::Role;

    if (type_ == WorkspaceTreeModel::Type::File) {
        switch (role) {
            case Qt::DisplayRole:
                return caption_;
            case Qt::ToolTipRole: {
                // prevent line breaks in the tooltip
                // see https://doc.qt.io/qt-5/qtooltip.html#details
                return QString("<p style='white-space:pre'><strong>%1</strong><br>%2</p>")
                    .arg(file_)
                    .arg(path_);
            }
            case static_cast<int>(Role::Type):
                return static_cast<int>(type_);
            case static_cast<int>(Role::Name):
                return file_;
            case static_cast<int>(Role::Path):
                return path_;
            case static_cast<int>(Role::FilePath):
                return path_ + "/" + file_;
            case static_cast<int>(Role::isExample):
                return isExample_;

            case static_cast<int>(Role::Title):
                infoLoader_->submit();
                return info_.title;
            case static_cast<int>(Role::Author):
                infoLoader_->submit();
                return info_.author;
            case static_cast<int>(Role::Tags):
                infoLoader_->submit();
                return info_.tags;
            case static_cast<int>(Role::Categories):
                infoLoader_->submit();
                return info_.categories;
            case static_cast<int>(Role::Description):
                infoLoader_->submit();
                return info_.description;
            case static_cast<int>(Role::Processors):
                infoLoader_->submit();
                return info_.processors;
            case static_cast<int>(Role::Image): {
                infoLoader_->submit();
                if (info_.image.isNull()) {
                    return QImage{":/inviwo/inviwo_light.png"};
                } else {
                    return info_.image;
                }
            }

            default:
                return {};
        }
    } else {
        switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
                return caption_;
            case static_cast<int>(WorkspaceTreeModel::Role::Type):
                return static_cast<int>(type_);
            default:
                return {};
        }
    }
}  // namespace inviwo

Qt::ItemFlags TreeItem::flags() const {
    if (type_ == WorkspaceTreeModel::Type::File) {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;
    } else {
        return Qt::NoItemFlags;
    }
}

WorkspaceTreeModel::Type TreeItem::type() const { return type_; }

bool TreeItem::operator==(const TreeItem& tree) const {
    bool match = (type_ == tree.type_) && (caption_ == tree.caption_) &&
                 (isExample_ == tree.isExample_) && (childCount() == tree.childCount());
    // check children
    if (match) {
        for (int i = 0; i < childCount() && match; ++i) {
            match &= children_[i].get()->operator==(*tree.child(i));
        }
    }

    return match;
}
bool TreeItem::operator!=(const TreeItem& tree) const { return !operator==(tree); }


WorkspaceTreeModel::WorkspaceTreeModel(InviwoApplication* app, QObject* parent)
    : QAbstractItemModel(parent), app_(app), root_{std::make_unique<TreeItem>(nullptr)} {

    addEntry(root_.get(), std::make_unique<TreeItem>(recent, Type::SubSection));
    addEntry(root_.get(), std::make_unique<TreeItem>(examples, Type::Section));
    addEntry(root_.get(), std::make_unique<TreeItem>(tests, Type::Section));
}

WorkspaceTreeModel::~WorkspaceTreeModel() = default;

QModelIndex WorkspaceTreeModel::index(int row, int column, const QModelIndex& parent) const {
    if (parent.isValid() && parent.column() != 0) return QModelIndex();

    TreeItem* parentItem = getItem(parent);
    TreeItem* childItem = parentItem->child(row);

    if (childItem) return createIndex(row, column, childItem);
    return QModelIndex();
}

Qt::ItemFlags WorkspaceTreeModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) return Qt::NoItemFlags;

    auto item = static_cast<TreeItem*>(index.internalPointer());
    return item->flags();
}

QVariant WorkspaceTreeModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return QVariant();
    auto item = static_cast<TreeItem*>(index.internalPointer());

    return item->data(index.column(), role);
}

QVariant WorkspaceTreeModel::headerData(int, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) return "Workspaces";

    return QVariant();
}

QModelIndex WorkspaceTreeModel::parent(const QModelIndex& index) const {
    if (!index.isValid()) return {};

    if (auto item = getItem(index)) {
        if (auto parent = item->parent()) {
            if (parent != root_.get()) {
                return createIndex(parent->row(), 0, parent);
            }
        }
    }
    return {};
}

int WorkspaceTreeModel::rowCount(const QModelIndex& parent) const {
    const TreeItem* parentItem = getItem(parent);
    return parentItem ? parentItem->childCount() : 0;
}

int WorkspaceTreeModel::columnCount(const QModelIndex& parent) const {
    return root_->columnCount();
}

void WorkspaceTreeModel::updateCategory(TreeItem* item,
                                        std::vector<std::unique_ptr<TreeItem>> children) {
    if (!item) return;

    // if children are identical, skip update
    if (children.size() == static_cast<size_t>(item->childCount())) {
        bool match = true;
        for (int i = 0; i < item->childCount() && match; ++i) {
            match &= (*children[i].get() == *item->child(i));
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

void WorkspaceTreeModel::addEntry(TreeItem* parent, std::unique_ptr<TreeItem> child) {
    QModelIndex index = getIndex(parent);
    int pos = parent->childCount();
    beginInsertRows(index, pos, pos);
    parent->addChild(std::move(child));
    endInsertRows();
}

QModelIndex WorkspaceTreeModel::getIndex(TreeItem* item, int column) const {
    if (!item) return QModelIndex();

    return createIndex(item->row(), column, item);
}

TreeItem* WorkspaceTreeModel::getItem(const QModelIndex& index) const {
    if (index.isValid()) {
        if (auto item = static_cast<TreeItem*>(index.internalPointer())) {
            return item;
        }
    }
    return root_.get();
}

TreeItem* WorkspaceTreeModel::getCategory(std::string_view caption) const {
    return root_->findChild(caption);
}

QModelIndex WorkspaceTreeModel::getCategoryIndex(std::string_view category) {
    if (auto item = getCategory(category)) {
        return getIndex(item);
    } else {
        return QModelIndex{};
    }
}

void WorkspaceTreeModel::updateRecentWorkspaces(const QStringList& recentFiles) {
    std::vector<std::unique_ptr<TreeItem>> items;
    for (auto& elem : recentFiles) {
        const std::string filename = utilqt::fromQString(elem);
        if (filesystem::fileExists(filename)) {
            items.push_back(std::make_unique<TreeItem>(
                filename, app_,
                [this](TreeItem* item) {
                    auto id = getIndex(item);
                    dataChanged(id, id);
                },
                false));
        }
    }
    updateCategory(getCategory(recent), std::move(items));
}

void WorkspaceTreeModel::updateModules(std::string_view category, ModulePath pathType,
                                       bool recursive) {

    auto addFile = [this](const std::string& filePath, TreeItem& section) {
        if (filesystem::getFileExtension(filePath) != "inv") return;
        section.addChild(std::make_unique<TreeItem>(
            filePath, app_,
            [this](TreeItem* item) {
                auto id = getIndex(item);
                dataChanged(id, id);
            },
            true));
    };

    std::vector<std::unique_ptr<TreeItem>> items;
    for (const auto& module : app_->getModules()) {
        auto path = module->getPath(pathType);
        if (!filesystem::directoryExists(path)) continue;

        auto section = std::make_unique<TreeItem>(module->getIdentifier(), Type::SubSection);
        if (recursive) {
            for (auto filePath : filesystem::getDirectoryContentsRecursively(path)) {
                addFile(filePath, *section);
            }
        } else {
            for (auto filePath : filesystem::getDirectoryContents(path)) {
                addFile(fmt::format("{}/{}", path, filePath), *section);
            }
        }
        if (section->childCount() > 0) {
            items.push_back(std::move(section));
        }
    }
    updateCategory(getCategory(category), std::move(items));
}

void WorkspaceTreeModel::updateExampleEntries() {
    updateModules(examples, ModulePath::Workspaces, false);
}

void WorkspaceTreeModel::updateRegressionTestEntries() {
    updateModules(tests, ModulePath::RegressionTests, true);
}

}  // namespace inviwo
