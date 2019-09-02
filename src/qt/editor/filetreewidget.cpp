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

    if (index.data(FileTreeModel::ItemRoles::Type).toInt() == FileTreeModel::ListElemType::File) {
        auto option = o;
        initStyleOption(&option, index);

        option.text = "";
        QStyle* style = option.widget ? option.widget->style() : QApplication::style();
        style->drawControl(QStyle::CE_ItemViewItem, &option, painter, option.widget);

        if (index.column() > 0) {
            const auto filename = index.data(FileTreeModel::ItemRoles::FileName).toString();
            const auto path = index.data(FileTreeModel::ItemRoles::Path).toString();
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

    if (index.data(FileTreeModel::ItemRoles::Type).toInt() == FileTreeModel::ListElemType::File) {
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

    const auto filename = index.data(FileTreeModel::ItemRoles::FileName).toString();
    const auto path = index.data(FileTreeModel::ItemRoles::Path).toString();

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
    , model_{new FileTreeModel{this}}
    , proxyModel_{new QSortFilterProxyModel{this}}
    , fileIcon_{":/inviwo/inviwo_light.png"} {

    proxyModel_->setSourceModel(model_);
    proxyModel_->setRecursiveFilteringEnabled(true);
    proxyModel_->setFilterCaseSensitivity(Qt::CaseInsensitive);
    setModel(proxyModel_);

    setHeaderHidden(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setIconSize(utilqt::emToPx(this, QSize(3, 3)));
    setIndentation(utilqt::emToPx(this, 1.0));
    setItemDelegate(new SectionDelegate(this));

    // adjust width of first column
    // file entries and icons start in column 2, sections headers span all columns
    header()->setMinimumSectionSize(0);
    header()->resizeSection(0, utilqt::emToPx(this, 2.0));

    QObject::connect(selectionModel(), &QItemSelectionModel::currentRowChanged, this,
                     [this](const QModelIndex& current, const QModelIndex&) {
                         if (current.isValid() && (current.data(FileTreeModel::ItemRoles::Type) ==
                                                   FileTreeModel::ListElemType::File)) {
                             const auto filename =
                                 current.data(FileTreeModel::ItemRoles::Path).toString() + "/" +
                                 current.data(FileTreeModel::ItemRoles::FileName).toString();
                             const auto isExample =
                                 current.data(FileTreeModel::ItemRoles::ExampleWorkspace).toBool();
                             emit selectedFileChanged(filename, isExample);
                         } else {
                             emit selectedFileChanged("", false);
                         }
                     });

    QObject::connect(this, &QTreeView::doubleClicked, this, [this](const QModelIndex& index) {
        if (index.isValid() &&
            (index.data(FileTreeModel::ItemRoles::Type) == FileTreeModel::ListElemType::File)) {
            const auto filename = index.data(FileTreeModel::ItemRoles::Path).toString() + "/" +
                                  index.data(FileTreeModel::ItemRoles::FileName).toString();
            const auto isExample = index.data(FileTreeModel::ItemRoles::ExampleWorkspace).toBool();
            emit loadFile(filename, isExample);
        }
    });
}

void FileTreeWidget::updateRecentWorkspaces(const QStringList& recentFiles) {
    if (!recentWorkspaceItem_) {
        auto item =
            std::make_unique<TreeItem>("Recent Workspaces", FileTreeModel::ListElemType::Section);
        recentWorkspaceItem_ = item.get();

        model_->addEntry(nullptr, std::move(item));
        auto index = proxyModel_->mapFromSource(model_->getIndex(recentWorkspaceItem_));
        expand(index);
        setFirstColumnSpanned(index.row(), index.parent(), true);
    }
    std::vector<std::unique_ptr<TreeItem>> items;
    for (auto& elem : recentFiles) {
        const std::string filename = utilqt::fromQString(elem);
        if (filesystem::fileExists(filename)) {
            items.push_back(std::make_unique<TreeItem>(fileIcon_, filename));
        }
    }
    setUpdatesEnabled(false);
    model_->updateCategory(recentWorkspaceItem_, std::move(items));
    setUpdatesEnabled(true);
}

void FileTreeWidget::updateExampleEntries() {
    std::vector<std::unique_ptr<TreeItem>> examples;
    for (const auto& module : inviwoApp_->getModules()) {
        auto moduleWorkspacePath = module->getPath(ModulePath::Workspaces);
        if (!filesystem::directoryExists(moduleWorkspacePath)) continue;

        auto category = std::make_unique<TreeItem>(utilqt::toQString(module->getIdentifier()),
                                                   FileTreeModel::ListElemType::SubSection);
        for (auto item : filesystem::getDirectoryContents(moduleWorkspacePath)) {
            // only accept inviwo workspace files
            if (filesystem::getFileExtension(item) != "inv") continue;
            category->addChild(
                std::make_unique<TreeItem>(fileIcon_, moduleWorkspacePath + "/" + item, true));
        }
        if (category->childCount() > 0) {
            examples.push_back(std::move(category));
        }
    }

    if (!examplesItem_) {
        auto item = std::make_unique<TreeItem>("Examples", FileTreeModel::ListElemType::Section);
        examplesItem_ = item.get();
        model_->addEntry(nullptr, std::move(item));
        auto index = proxyModel_->mapFromSource(model_->getIndex(examplesItem_));
        expand(index);
        setFirstColumnSpanned(index.row(), index.parent(), true);
    }
    setRowHidden(examplesItem_->row(), QModelIndex(), examples.empty());

    if (!examples.empty()) {
        setUpdatesEnabled(false);
        model_->updateCategory(examplesItem_, std::move(examples));

        for (int i = 0; i < examplesItem_->childCount(); ++i) {
            TreeItem* item = examplesItem_->child(i);
            auto index = proxyModel_->mapFromSource(model_->getIndex(item));
            expand(index);
            setFirstColumnSpanned(index.row(), index.parent(), true);
        }
        setUpdatesEnabled(true);
    }
}

void FileTreeWidget::updateRegressionTestEntries() {
    std::vector<std::unique_ptr<TreeItem>> tests;
    for (const auto& module : inviwoApp_->getModules()) {
        auto moduleRegressionTestsPath = module->getPath(ModulePath::RegressionTests);
        if (!filesystem::directoryExists(moduleRegressionTestsPath)) continue;

        auto category = std::make_unique<TreeItem>(utilqt::toQString(module->getIdentifier()),
                                                   FileTreeModel::ListElemType::SubSection);

        std::vector<TreeItem*> moduleTests;
        for (auto item : filesystem::getDirectoryContentsRecursively(moduleRegressionTestsPath)) {
            // only accept inviwo workspace files
            if (filesystem::getFileExtension(item) != "inv") continue;
            category->addChild(std::make_unique<TreeItem>(fileIcon_, item, true));
        }
        if (category->childCount() > 0) {
            tests.push_back(std::move(category));
        }
    }
    if (!regressionTestsItem_) {
        auto item =
            std::make_unique<TreeItem>("Regression Tests", FileTreeModel::ListElemType::Section);
        regressionTestsItem_ = item.get();
        model_->addEntry(nullptr, std::move(item));
        auto index = proxyModel_->mapFromSource(model_->getIndex(regressionTestsItem_));
        collapse(index);
        setFirstColumnSpanned(index.row(), index.parent(), true);
    }
    setRowHidden(regressionTestsItem_->row(), QModelIndex(), tests.empty());

    if (!tests.empty()) {
        setUpdatesEnabled(false);
        model_->updateCategory(regressionTestsItem_, std::move(tests));

        for (int i = 0; i < regressionTestsItem_->childCount(); ++i) {
            TreeItem* item = regressionTestsItem_->child(i);
            auto index = proxyModel_->mapFromSource(model_->getIndex(item));
            expand(index);
            setFirstColumnSpanned(index.row(), index.parent(), true);
        }
        setUpdatesEnabled(true);
    }
}

bool FileTreeWidget::selectRecentWorkspace(int index) {
    if (!recentWorkspaceItem_) return false;
    if (recentWorkspaceItem_->childCount() < index) return false;

    auto idx = proxyModel_->mapFromSource(model_->getIndex(recentWorkspaceItem_->child(index)));
    selectionModel()->setCurrentIndex(
        idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

    return true;
}

void FileTreeWidget::setFilter(const QString& str) {
    proxyModel_->setFilterRegExp(str);

    expandItems();

    // select first leaf node
    auto index = findFirstLeaf(QModelIndex());
    if (index.isValid()) {
        selectionModel()->setCurrentIndex(
            index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }
}

void FileTreeWidget::expandItems() {
    // fold/unfold all tree items based on filtering
    if (proxyModel_->filterRegExp().isEmpty()) {
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
void FileTreeWidget::expandRecursively(const QModelIndex& index) {
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

QModelIndex FileTreeWidget::findFirstLeaf(QModelIndex parent) const {
    if (!proxyModel_->hasChildren(parent)) return parent;
    return findFirstLeaf(proxyModel_->index(0, 0, parent));
}

void FileTreeWidget::defaultExpand() {
    setUpdatesEnabled(false);

    expandRecursively(proxyModel_->mapFromSource(model_->getIndex(recentWorkspaceItem_)));
    expandRecursively(proxyModel_->mapFromSource(model_->getIndex(examplesItem_)));

    auto index = proxyModel_->mapFromSource(model_->getIndex(regressionTestsItem_));
    expandRecursively(index);
    collapse(index);

    setUpdatesEnabled(true);
}

}  // namespace inviwo
