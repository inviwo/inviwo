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

#include <inviwo/qt/editor/workspacetreeview.h>

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
#include <QScrollBar>
#include <warn/pop>

namespace inviwo {

using Role = WorkspaceModelRole;
using Type = WorkspaceModelType;

namespace {

class SectionDelegate : public QStyledItemDelegate {
public:
    SectionDelegate(QWidget* parent = nullptr);
    virtual ~SectionDelegate() override = default;

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option,
                       const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    static QString elidedText(const QString& str, const QFontMetrics& metrics, int width);
};

SectionDelegate::SectionDelegate(QWidget* parent) : QStyledItemDelegate(parent) {}

void SectionDelegate::paint(QPainter* painter, const QStyleOptionViewItem& o,
                            const QModelIndex& index) const {

    if (utilqt::getData(index, Role::Type) == Type::File) {
        auto option = o;
        initStyleOption(&option, index);
        painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
                                QPainter::SmoothPixmapTransform);

        option.text = "";
        QStyle* style = option.widget ? option.widget->style() : QApplication::style();
        style->drawControl(QStyle::CE_ItemViewItem, &option, painter, option.widget);

        const auto name = utilqt::getData(index, Role::Name).toString();
        const auto path = utilqt::getData(index, Role::Path).toString();
        const auto image = utilqt::getData(index, Role::PrimaryImage).value<QImage>();

        // draw text
        painter->save();

        const auto fm = QFontMetrics(option.font);
        const auto margin = utilqt::emToPx(fm, 0.5);
        const auto margins = QMargins(margin, margin, margin, margin);

        auto imgRect = option.rect;
        auto imageWidth = imgRect.height();

        imgRect.setWidth(imageWidth);
        imgRect = imgRect.marginsRemoved(margins);

        QPainterPath border;
        border.addRoundedRect(imgRect, 35, 35, Qt::RelativeSize);

        const auto imageCenter = image.rect().center();
        const auto imageSize = std::min(image.rect().width(), image.rect().height());
        const QRect sourceRect{imageCenter - QPoint{imageSize, imageSize} / 2,
                               imageCenter + QPoint{imageSize, imageSize} / 2};

        painter->setClipPath(border);
        painter->setClipping(true);
        painter->drawImage(imgRect, image, sourceRect);
        painter->setClipping(false);

        painter->setPen(QPen{option.palette.text().color(), 1.5});
        painter->drawPath(border);

        auto nameRect = option.rect;
        nameRect.adjust(imageWidth, margin, 0, -imageWidth / 2);
        auto pathRect = option.rect;
        pathRect.adjust(imageWidth, imageWidth / 2, 0, -margin);

        auto fontFilename = option.font;
        fontFilename.setBold(true);
        painter->setFont(fontFilename);
        painter->setPen(option.palette.text().color().lighter());
        painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, name);

        painter->setFont(option.font);
        painter->setPen(option.palette.text().color());
        painter->drawText(pathRect, Qt::AlignLeft | Qt::AlignVCenter,
                          elidedText(path, option.fontMetrics, pathRect.width()));
        painter->restore();

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

    if (utilqt::getData(index, Role::Type) == Type::File) {
        auto option = o;
        initStyleOption(&option, index);

        const auto fm = QFontMetrics(option.font);
        return QSize{option.rect.width(), utilqt::emToPx(fm, 4)};

    } else {
        return QStyledItemDelegate::sizeHint(o, index);
    }
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

WorkspaceTreeView::WorkspaceTreeView(QAbstractItemModel* theModel, QWidget* parent)
    : QTreeView{parent} {

    setModel(theModel);

    setHeaderHidden(true);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    setIndentation(utilqt::emToPx(this, 1.0));
    setItemDelegate(new SectionDelegate(this));

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
}

void WorkspaceTreeView::collapseRecursively(const QModelIndex& index) {
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
