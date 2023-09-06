/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2023 Inviwo Foundation
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
#include <inviwo/core/common/modulemanager.h>

#include <modules/qtwidgets/inviwoqtutils.h>
#include <inviwo/qt/editor/workspaceannotationsqt.h>
#include <modules/qtwidgets/editorsettings.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QFontMetrics>
#include <warn/pop>

namespace {

struct RegisterWorkspaceInfo {
    RegisterWorkspaceInfo() { qRegisterMetaType<inviwo::WorkspaceInfo>(); }
} registerWorkspaceInfo;

}  // namespace

namespace inviwo {

class TreeItem : public QObject {
public:
    static const int IconSize = 12;  // Icon size in em.

    explicit TreeItem(TreeItem* parent);
    TreeItem(std::string_view caption, WorkspaceTreeModel::Type type);
    TreeItem(const std::filesystem::path& filename, InviwoApplication* app,
             std::function<void(TreeItem*)> onChange, bool readOnly);

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
        auto annotations = std::make_shared<WorkspaceAnnotationsQt>(filename_, app_);
        emit workspaceInfoLoaded(WorkspaceInfo{annotations});
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

TreeItem::TreeItem(const std::filesystem::path& filename, InviwoApplication* app,
                   std::function<void(TreeItem*)> onChange, bool isExample)
    : type_{WorkspaceTreeModel::Type::File}
    , parent_{nullptr}
    , row_{0}
    , caption_{utilqt::toQString(filename.stem())}
    , file_{utilqt::toQString(filename.filename())}
    , path_{utilqt::toQString(filename.parent_path())}
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
                if (path_.endsWith("/")) {
                    // path already ends with '/' in case of root directory '/' or disk drive 'C:/'
                    return path_ + file_;
                } else {
                    return path_ + "/" + file_;
                }
            case static_cast<int>(Role::isExample):
                return isExample_;
            case static_cast<int>(Role::Annotations):
                infoLoader_->submit();
                if (!info_.annotations) return {};
                return QVariant::fromValue(info_);

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
    addEntry(root_.get(), std::make_unique<TreeItem>(restore, Type::SubSection));
    addEntry(root_.get(), std::make_unique<TreeItem>(custom, Type::Section));
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

int WorkspaceTreeModel::columnCount(const QModelIndex&) const { return root_->columnCount(); }

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
        const std::filesystem::path filename = utilqt::toPath(elem);
        if (std::filesystem::is_regular_file(filename)) {
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

void WorkspaceTreeModel::updateRestoreEntries() {
    const auto restores = filesystem::getPath(PathType::Settings) / "autosaves";

    if (!std::filesystem::is_directory(restores)) return;

    std::filesystem::directory_iterator it(restores);

    std::vector<std::filesystem::path> files{};
    std::copy_if(it, end(it), std::back_inserter(files), [](const std::filesystem::path& path) {
        return path.extension().string() == ".inv";
    });

    std::sort(files.begin(), files.end(),
              [](const std::filesystem::path& a, const std::filesystem::path& b) {
                  return std::filesystem::last_write_time(a) > std::filesystem::last_write_time(b);
              });

    std::vector<std::unique_ptr<TreeItem>> items;

    std::transform(files.begin(), files.end(), std::back_inserter(items),
                   [&](const std::filesystem::path& path) {
                       return std::make_unique<TreeItem>(
                           path, app_,
                           [this](TreeItem* item) {
                               auto id = getIndex(item);
                               dataChanged(id, id);
                           },
                           true);
                   });

    updateCategory(getCategory(restore), std::move(items));
}

void WorkspaceTreeModel::updateCustomEntries() {
    const auto& props = app_->getSettingsByType<EditorSettings>()->workspaceDirectories;

    std::vector<std::unique_ptr<TreeItem>> items;
    for (const auto* fileProp : props.getPropertiesByType<FileProperty>()) {
        auto& path = fileProp->get();

        if (!std::filesystem::is_directory(path)) continue;

        auto section = std::make_unique<TreeItem>(path.generic_string(), Type::SubSection);

        std::filesystem::directory_iterator it(path);

        std::vector<std::filesystem::path> files{};
        std::copy_if(it, end(it), std::back_inserter(files), [](const std::filesystem::path& path) {
            return path.extension().string() == ".inv";
        });

        std::sort(files.begin(), files.end(),
                  [](const std::filesystem::path& a, const std::filesystem::path& b) {
                      return std::filesystem::last_write_time(a) >
                             std::filesystem::last_write_time(b);
                  });

        std::for_each(files.begin(), files.end(), [&](const std::filesystem::path& path) {
            section->addChild(std::make_unique<TreeItem>(
                path, app_,
                [this](TreeItem* item) {
                    auto id = getIndex(item);
                    dataChanged(id, id);
                },
                false));
        });
        if (section->childCount() > 0) {
            items.push_back(std::move(section));
        }
    }
    updateCategory(getCategory(custom), std::move(items));
}

void WorkspaceTreeModel::updateModules(std::string_view category, ModulePath pathType,
                                       bool recursive) {

    auto addFile = [this](const std::filesystem::path& filePath, TreeItem& section) {
        if (filePath.extension() != ".inv") return;
        section.addChild(std::make_unique<TreeItem>(
            filePath, app_,
            [this](TreeItem* item) {
                auto id = getIndex(item);
                dataChanged(id, id);
            },
            true));
    };

    std::vector<std::unique_ptr<TreeItem>> items;
    for (const auto& inviwoModule : app_->getModuleManager().getInviwoModules()) {
        auto path = inviwoModule.getPath(pathType);
        if (!std::filesystem::is_directory(path)) continue;

        auto section = std::make_unique<TreeItem>(inviwoModule.getIdentifier(), Type::SubSection);
        if (recursive) {
            for (auto filePath : filesystem::getDirectoryContentsRecursively(path)) {
                addFile(path / filePath, *section);
            }
        } else {
            for (auto filePath : filesystem::getDirectoryContents(path)) {
                addFile(path / filePath, *section);
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
