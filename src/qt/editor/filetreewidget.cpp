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

#include <inviwo/qt/editor/filetreewidget.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwomodule.h>
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

#include <warn/pop>

namespace inviwo {

TreeItem::TreeItem(TreeItem* parent) : parent_{parent} {}

TreeItem::TreeItem(const QString& caption, FileTreeWidget::ListElemType type, TreeItem* parent)
    : parent_{parent}, type_{type}, caption_{caption} {}

TreeItem::TreeItem(const QIcon& icon, const std::string& filename, bool isExample, TreeItem* parent)
    : parent_{parent} {
    setData(icon, filename, isExample);
}

TreeItem::~TreeItem() { removeChildren(); }

void TreeItem::addChild(TreeItem* child) {
    childItems_.push_back(child);
    child->parent_ = this;
}

void TreeItem::addChildren(std::vector<TreeItem*> children) {
    for (auto child : children) {
        child->parent_ = this;
    }
    util::append(childItems_, children);
}

bool TreeItem::insertChildren(int position, int count) {
    if (position < 0 || position > childItems_.size()) return false;

    for (int row = 0; row < count; ++row) {
        TreeItem* item = new TreeItem(this);
        childItems_.insert(childItems_.begin() + position, item);
    }

    return true;
}

bool TreeItem::removeChildren(int position, int count) {
    if (position < 0 || position + count > childItems_.size()) return false;

    for (int row = 0; row < count; ++row) delete childItems_[position];
    childItems_.erase(childItems_.begin() + position);

    return true;
}

void TreeItem::removeChildren() {
    for (auto c : childItems_) {
        delete c;
    }
}

TreeItem* TreeItem::child(int row) {
    if ((row < 0) || (row >= childItems_.size())) return nullptr;
    return childItems_[row];
}

int TreeItem::row() const {
    if (parent_) {
        auto it = util::find(parent_->childItems_, const_cast<TreeItem*>(this));
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

    if (type_ == FileTreeWidget::ListElemType::File) {
        switch (role) {
            case Qt::DisplayRole:
                if (column == 0) {
                    return {};
                } else {
                    return caption_;
                }
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
            case FileTreeWidget::ItemRoles::Type:
                return type_;
            case FileTreeWidget::ItemRoles::FileName:
                return file_;
            case FileTreeWidget::ItemRoles::Path:
                return path_;
            case FileTreeWidget::ItemRoles::ExampleWorkspace:
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
            case FileTreeWidget::ItemRoles::Type:
                return type_;
            default:
                return {};
        }
    }
}

FileTreeWidget::ListElemType TreeItem::type() const { return type_; }

void TreeItem::setData(const QString& caption, FileTreeWidget::ListElemType type) {
    type_ = type;
    caption_ = caption;
}

void TreeItem::setData(const QIcon& icon, const std::string& filename, bool isExample) {
    type_ = FileTreeWidget::ListElemType::File;

    icon_ = icon;
    caption_ = utilqt::toQString(filename);
    file_ = utilqt::toQString(filesystem::getFileNameWithExtension(filename));
    path_ = utilqt::toQString(filesystem::getFileDirectory(filename));
    isExample_ = isExample;

    if (path_.isEmpty()) {
        path_ = ".";
    }
}

TreeModel::TreeModel(QObject* parent) : QAbstractItemModel(parent), root_{new TreeItem(nullptr)} {}

TreeModel::~TreeModel() { delete root_; }

QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const {
    if (parent.isValid() && parent.column() != 0) return QModelIndex();

    TreeItem* parentItem = getItem(parent);
    TreeItem* childItem = parentItem->child(row);

    if (childItem) return createIndex(row, column, childItem);
    return QModelIndex();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index);
}

QVariant TreeModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return QVariant();
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());

    return item->data(index.column(), role);
}

QVariant TreeModel::headerData(int, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) return "Workspaces";

    return QVariant();
}

QModelIndex TreeModel::parent(const QModelIndex& index) const {
    if (!index.isValid()) return QModelIndex();

    TreeItem* childItem = getItem(index);
    TreeItem* parentItem = childItem ? childItem->parent() : nullptr;

    if (parentItem == root_ || !parentItem) return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex& parent) const {
    const TreeItem* parentItem = getItem(parent);
    return parentItem ? parentItem->childCount() : 0;
}

int TreeModel::columnCount(const QModelIndex&) const { return root_->columnCount(); }

bool TreeModel::insertRows(int position, int rows, const QModelIndex& parent) {
    TreeItem* parentItem = getItem(parent);
    if (!parentItem) return false;

    beginInsertRows(parent, position, position + rows - 1);
    const bool success = parentItem->insertChildren(position, rows);
    endInsertRows();

    return success;
}

bool TreeModel::removeRows(int position, int rows, const QModelIndex& parent) {
    TreeItem* parentItem = getItem(parent);
    if (!parentItem) return false;

    beginRemoveRows(parent, position, position + rows - 1);
    const bool success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

void TreeModel::updateCategory(TreeItem* item, std::vector<TreeItem*> children) {
    QModelIndex index = getIndex(item);

    beginRemoveRows(index, 0, item->childCount());

    endRemoveRows();
    beginInsertRows(index, 0, static_cast<int>(children.size()));
    for (auto c : children) {
        item->addChild(c);
    }
    endInsertRows();
}

void TreeModel::addEntry(TreeItem* node, TreeItem* child) {
    QModelIndex index = getIndex(node);
    int pos = node ? node->childCount() : rowCount();
    beginInsertRows(index, pos, pos + 1);

    if (node) {
        node->addChild(child);
    } else {
        if (!root_) {
            root_ = new TreeItem();
        }
        root_->addChild(child);
    }
    endInsertRows();
}

bool TreeModel::removeEntry(TreeItem* node) {
    return removeRows(node->row(), 1, getIndex(node->parent()));
}

bool TreeModel::removeChildren(TreeItem* root) {
    if (!root || root == root_) {
        beginResetModel();
        root_->removeChildren();
        endResetModel();
        return true;
    } else {
        return removeRows(0, root->childCount(), getIndex(root));
    }
}

QModelIndex TreeModel::getIndex(TreeItem* item, int column) const {
    if (!item) return QModelIndex();

    return createIndex(item->row(), column, item);
}

TreeItem* TreeModel::getItem(const QModelIndex& index) const {
    if (index.isValid()) {
        TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
        if (item) return item;
    }
    return root_;
}

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

    if (index.data(FileTreeWidget::ItemRoles::Type).toInt() == FileTreeWidget::ListElemType::File) {
        auto option = o;
        initStyleOption(&option, index);

        option.text = "";
        QStyle* style = option.widget ? option.widget->style() : QApplication::style();
        style->drawControl(QStyle::CE_ItemViewItem, &option, painter, option.widget);

        if (index.column() > 0) {
            const auto filename = index.data(FileTreeWidget::ItemRoles::FileName).toString();
            const auto path = index.data(FileTreeWidget::ItemRoles::Path).toString();
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

    if (index.data(FileTreeWidget::ItemRoles::Type).toInt() == FileTreeWidget::ListElemType::File) {
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

    const auto filename = index.data(FileTreeWidget::ItemRoles::FileName).toString();
    const auto path = index.data(FileTreeWidget::ItemRoles::Path).toString();

    auto fontFilename = option.font;
    fontFilename.setBold(true);
    const auto fm = QFontMetrics(fontFilename);

    const int marginLeft = utilqt::emToPx(fm, 6.0 / utilqt::refEm());
    const int textSpacing = utilqt::emToPx(fm, 4.0 / utilqt::refEm());
    const int margin = utilqt::emToPx(fm, 10.0 / utilqt::refEm());
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

FileTreeWidget::FileTreeWidget(InviwoApplication* app, QWidget* parent)
    : QTreeView{parent}
    , inviwoApp_(app)
    , model_{new TreeModel{this}}
    , fileIcon_{":/inviwo/inviwo_light.png"} {

    setModel(model_);

    setHeaderHidden(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setIconSize(utilqt::emToPx(this, QSize(3, 3)));
    setIndentation(utilqt::emToPx(this, 1.0));
    setItemDelegate(new SectionDelegate(this));

    // adjust width of first column
    // file entries and icons start in column 2, sections headers span all columns
    header()->setMinimumSectionSize(0);
    header()->resizeSection(0, utilqt::emToPx(this, 1.0));

    QObject::connect(
        selectionModel(), &QItemSelectionModel::currentRowChanged, this,
        [this](const QModelIndex& current, const QModelIndex&) {
            if (current.isValid() && (current.data(ItemRoles::Type) == ListElemType::File)) {
                const auto filename = current.data(ItemRoles::Path).toString() + "/" +
                                      current.data(ItemRoles::FileName).toString();
                const auto isExample = current.data(ItemRoles::ExampleWorkspace).toBool();
                emit selectedFileChanged(filename, isExample);
            } else {
                emit selectedFileChanged("", false);
            }
        });

    QObject::connect(this, &QTreeView::doubleClicked, this, [this](const QModelIndex& index) {
        if (index.isValid() && (index.data(ItemRoles::Type) == ListElemType::File)) {
            const auto filename = index.data(ItemRoles::Path).toString() + "/" +
                                  index.data(ItemRoles::FileName).toString();
            const auto isExample = index.data(ItemRoles::ExampleWorkspace).toBool();
            emit loadFile(filename, isExample);
        }
    });
}

void FileTreeWidget::updateRecentWorkspaces(const QStringList& recentFiles) {
    if (!recentWorkspaceItem_) {
        recentWorkspaceItem_ = new TreeItem("Recent Workspaces", ListElemType::Section);

        model_->addEntry(nullptr, recentWorkspaceItem_);
        auto index = model_->getIndex(recentWorkspaceItem_);
        expand(index);
        setFirstColumnSpanned(index.row(), index.parent(), true);
    }
    std::vector<TreeItem*> items;
    for (auto& elem : recentFiles) {
        const std::string filename = utilqt::fromQString(elem);
        if (filesystem::fileExists(filename)) {
            items.push_back(new TreeItem{fileIcon_, filename});
        }
    }
    setUpdatesEnabled(false);
    model_->updateCategory(recentWorkspaceItem_, items);
    setUpdatesEnabled(true);
}

void FileTreeWidget::updateExampleEntries() {
    std::vector<TreeItem*> examples;
    for (const auto& module : inviwoApp_->getModules()) {
        auto moduleWorkspacePath = module->getPath(ModulePath::Workspaces);
        if (!filesystem::directoryExists(moduleWorkspacePath)) continue;
        std::vector<TreeItem*> moduleExamples;
        for (auto item : filesystem::getDirectoryContents(moduleWorkspacePath)) {
            // only accept inviwo workspace files
            if (filesystem::getFileExtension(item) != "inv") continue;
            moduleExamples.emplace_back(
                new TreeItem{fileIcon_, moduleWorkspacePath + "/" + item, true});
        }
        if (!moduleExamples.empty()) {
            auto category =
                new TreeItem{utilqt::toQString(module->getIdentifier()), ListElemType::SubSection};
            category->addChildren(moduleExamples);
            examples.push_back(category);
        }
    }

    if (!examplesItem_) {
        examplesItem_ = new TreeItem("Examples", ListElemType::Section);
        model_->addEntry(nullptr, examplesItem_);
        auto index = model_->getIndex(examplesItem_);
        expand(index);
        setFirstColumnSpanned(index.row(), index.parent(), true);
    }
    setRowHidden(examplesItem_->row(), QModelIndex(), examples.empty());

    if (!examples.empty()) {
        setUpdatesEnabled(false);
        model_->updateCategory(examplesItem_, examples);

        for (auto elem : examples) {
            auto index = model_->getIndex(elem);
            expand(index);
            setFirstColumnSpanned(index.row(), index.parent(), true);
        }
        setUpdatesEnabled(true);
    }
}

void FileTreeWidget::updateRegressionTestEntries() {
    std::vector<TreeItem*> tests;
    for (const auto& module : inviwoApp_->getModules()) {
        auto moduleRegressionTestsPath = module->getPath(ModulePath::RegressionTests);
        if (!filesystem::directoryExists(moduleRegressionTestsPath)) continue;
        std::vector<TreeItem*> moduleTests;
        for (auto item : filesystem::getDirectoryContentsRecursively(moduleRegressionTestsPath)) {
            // only accept inviwo workspace files
            if (filesystem::getFileExtension(item) != "inv") continue;
            moduleTests.push_back(new TreeItem{fileIcon_, item, true});
        }
        if (!moduleTests.empty()) {
            auto category =
                new TreeItem{utilqt::toQString(module->getIdentifier()), ListElemType::SubSection};
            category->addChildren(moduleTests);
            tests.push_back(category);
        }
    }
    if (!regressionTestsItem_) {
        regressionTestsItem_ = new TreeItem{"Regression Tests", ListElemType::Section};
        model_->addEntry(nullptr, regressionTestsItem_);
        auto index = model_->getIndex(regressionTestsItem_);
        collapse(index);
        setFirstColumnSpanned(index.row(), index.parent(), true);
    }
    setRowHidden(regressionTestsItem_->row(), QModelIndex(), tests.empty());

    if (!tests.empty()) {
        setUpdatesEnabled(false);
        model_->updateCategory(regressionTestsItem_, tests);

        for (auto elem : tests) {
            auto index = model_->getIndex(elem);
            expand(index);
            setFirstColumnSpanned(index.row(), index.parent(), true);
        }
        setUpdatesEnabled(true);
    }
}

bool FileTreeWidget::selectRecentWorkspace(int index) {
    if (!recentWorkspaceItem_) return false;
    if (recentWorkspaceItem_->childCount() < index) return false;

    auto idx = model_->index(index, 0, model_->index(recentWorkspaceItem_->row(), 0));
    selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

    return true;
}

}  // namespace inviwo
