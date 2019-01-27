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

namespace {

class SectionDelegate : public QStyledItemDelegate {
public:
    SectionDelegate(QWidget *parent = nullptr);
    virtual ~SectionDelegate() override = default;

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    std::tuple<QRect, QRect, QRect> getTextBoundingBox(const QStyleOptionViewItem &option,
                                                       const QModelIndex &index) const;

    static QString elidedText(const QString &str, const QFontMetrics &metrics, int width);
};

SectionDelegate::SectionDelegate(QWidget *parent) : QStyledItemDelegate(parent) {}

void SectionDelegate::paint(QPainter *painter, const QStyleOptionViewItem &o,
                            const QModelIndex &index) const {
    if (index.data(FileTreeWidget::ItemRoles::Type).toInt() == FileTreeWidget::ListElemType::File) {
        auto option = o;
        initStyleOption(&option, index);

        option.text = "";
        QStyle *style = option.widget ? option.widget->style() : QApplication::style();
        style->drawControl(QStyle::CE_ItemViewItem, &option, painter, option.widget);

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
        painter->drawText(std::get<2>(boundingRects), Qt::AlignLeft | Qt::AlignTop,
                          elidedText(path, option.fontMetrics, std::get<0>(boundingRects).width()));
        painter->restore();
    } else {
        QStyledItemDelegate::paint(painter, o, index);
    }
}

QSize SectionDelegate::sizeHint(const QStyleOptionViewItem &o, const QModelIndex &index) const {
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
    const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (!index.isValid()) return {};

    const int marginLeft = 6;
    const int textSpacing = 4;
    const int margin = 10;
    const int marginRight = 6;

    const auto filename = index.data(FileTreeWidget::ItemRoles::FileName).toString();
    const auto path = index.data(FileTreeWidget::ItemRoles::Path).toString();

    auto fontFilename = option.font;
    fontFilename.setBold(true);

    auto textRect = (option.rect.isValid() ? option.rect : QRect());
    textRect.adjust(marginLeft + option.decorationSize.width(), margin, -marginRight, 0);
    // set rect height to zero, since the font metric will calculate the required height of the text
    textRect.setHeight(0);

    auto filenameRect =
        QFontMetrics(fontFilename).boundingRect(textRect, Qt::AlignLeft | Qt::AlignTop, filename);

    textRect.setTop(filenameRect.bottom() + textSpacing);
    auto pathRect = option.fontMetrics.boundingRect(textRect, Qt::AlignLeft | Qt::AlignTop, path);

    auto boundingRect = filenameRect.united(pathRect);
    boundingRect.adjust(0, -margin, 0, margin);
    boundingRect.setRight(option.rect.right() - marginRight);

    return {boundingRect, filenameRect, pathRect};
}

QString SectionDelegate::elidedText(const QString &str, const QFontMetrics &metrics, int width) {
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
                   [&](const auto &dir) { return metrics.boundingRect("/" + dir).width(); });

    const int widthDots = metrics.boundingRect("/...").width();

    if (widthFirst + widthDots + widthDirs.front() > width) {
        // eliding path components is not sufficient, elide entire string
        return metrics.elidedText(str, Qt::ElideRight, width);
    }

    int leftWidth = width - widthFirst - widthDots;
    QString result =
        std::accumulate(directories.begin(), directories.end(), QString(),
                        [&, index = 0, leftWidth](QString str, const QString &dir) mutable {
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

FileTreeWidget::FileTreeWidget(InviwoApplication *app, QWidget *parent)
    : QTreeWidget{parent}, inviwoApp_(app), fileIcon_{":/inviwo/inviwo_light.png"} {
    
    setHeaderHidden(true);
    setColumnCount(2);
    setIconSize(QSize(24, 24));
    setIndentation(10);
    setColumnWidth(0, 10);
    setItemDelegate(new SectionDelegate(this));

    QObject::connect(
        this, &QTreeWidget::currentItemChanged, this,
        [this](QTreeWidgetItem *current, QTreeWidgetItem *) {
            if (current && (current->data(1, ItemRoles::Type) == ListElemType::File)) {
                const auto filename = current->data(1, ItemRoles::Path).toString() + "/" +
                                      current->data(1, ItemRoles::FileName).toString();
                const auto isExample = current->data(1, ItemRoles::ExampleWorkspace).toBool();
                emit selectedFileChanged(filename, isExample);
            } else {
                emit selectedFileChanged("", false);
            }
        });

    QObject::connect(
        this, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem *item, int) {
            if (item && (item->data(1, ItemRoles::Type) == ListElemType::File)) {
                const auto filename = item->data(1, ItemRoles::Path).toString() + "/" +
                                      item->data(1, ItemRoles::FileName).toString();
                const auto isExample = item->data(1, ItemRoles::ExampleWorkspace).toBool();
                emit loadFile(filename, isExample);
            }
        });
}

void FileTreeWidget::updateRecentWorkspaces(const QStringList &recentFiles) {
    if (!recentWorkspaceItem_) {
        recentWorkspaceItem_ = createCategory("Recent Workspaces");
        addTopLevelItem(recentWorkspaceItem_);
        recentWorkspaceItem_->setFirstColumnSpanned(true);
        recentWorkspaceItem_->setExpanded(true);
    }
    QList<QTreeWidgetItem *> items;
    for (auto &elem : recentFiles) {
        if (filesystem::fileExists(utilqt::fromQString(elem))) {
            items.append(createFileEntry(fileIcon_, utilqt::fromQString(elem)));
        }
    }
    setUpdatesEnabled(false);
    removeChildren(recentWorkspaceItem_);
    recentWorkspaceItem_->addChildren(items);
    setUpdatesEnabled(true);
}

void FileTreeWidget::updateExampleEntries() {
    QList<QTreeWidgetItem *> examples;
    for (const auto &module : inviwoApp_->getModules()) {
        auto moduleWorkspacePath = module->getPath(ModulePath::Workspaces);
        if (!filesystem::directoryExists(moduleWorkspacePath)) continue;
        QList<QTreeWidgetItem *> moduleExamples;
        for (auto item : filesystem::getDirectoryContents(moduleWorkspacePath)) {
            // only accept inviwo workspace files
            if (filesystem::getFileExtension(item) != "inv") continue;
            moduleExamples.append(
                createFileEntry(fileIcon_, moduleWorkspacePath + "/" + item, true));
        }
        if (!moduleExamples.isEmpty()) {
            auto category = createCategory(utilqt::toQString(module->getIdentifier()),
                                           ListElemType::SubSection);
            category->addChildren(moduleExamples);
            examples.push_back(category);
        }
    }
    if (!examplesItem_) {
        examplesItem_ = createCategory("Examples");
        addTopLevelItem(examplesItem_);
        examplesItem_->setFirstColumnSpanned(true);
        examplesItem_->setExpanded(true);
    }
    examplesItem_->setHidden(examples.isEmpty());

    if (!examples.isEmpty()) {
        setUpdatesEnabled(false);
        removeChildren(examplesItem_);
        examplesItem_->addChildren(examples);
        for (auto elem : examples) {
            elem->setExpanded(true);
            elem->setFirstColumnSpanned(true);
        }
        setUpdatesEnabled(true);
    }
}

bool FileTreeWidget::selectRecentWorkspace(int index) {
    if (!recentWorkspaceItem_) return false;
    if (recentWorkspaceItem_->childCount() < index) return false;

    setCurrentItem(recentWorkspaceItem_->child(index));
    return true;
}

QTreeWidgetItem *FileTreeWidget::createCategory(const QString &caption, ListElemType type) {
    auto item = new QTreeWidgetItem({caption}, type);
    item->setData(0, ItemRoles::Type, type);
    auto font = item->font(0);
    font.setBold(true);
    font.setPointSizeF(font.pointSize() * 1.2);
    item->setFont(0, font);
    return item;
}

QTreeWidgetItem *FileTreeWidget::createFileEntry(const QIcon &icon, const std::string &filename,
                                                 bool isExample) {
    auto file = filesystem::getFileNameWithExtension(filename);
    auto path = filesystem::getFileDirectory(filename);
    if (path.empty()) {
        path = ".";
    }
    auto item = new QTreeWidgetItem({"", utilqt::toQString(filename)}, ListElemType::File);
    item->setData(1, ItemRoles::Type, ListElemType::File);
    item->setData(1, ItemRoles::FileName, utilqt::toQString(file));
    item->setData(1, ItemRoles::Path, utilqt::toQString(path));
    item->setData(1, ItemRoles::ExampleWorkspace, isExample);
    item->setData(1, Qt::ToolTipRole, item->data(1, ItemRoles::Path));
    item->setData(1, Qt::DecorationRole, icon);
    return item;
}

void FileTreeWidget::removeChildren(QTreeWidgetItem *node) {
    if (node) {
        for (auto child : node->takeChildren()) {
            delete child;
        }
    }
}

}  // namespace inviwo
