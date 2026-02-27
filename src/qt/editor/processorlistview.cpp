/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwo/qt/editor/processorlistview.h>

#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/zip.h>

#include <inviwo/qt/editor/processorlistmodel.h>
#include <inviwo/qt/editor/processorlistwidget.h>
#include <inviwo/qt/editor/processormimedata.h>
#include <inviwo/qt/editor/processorpreview.h>

#include <modules/qtwidgets/inviwoqtutils.h>

#include <QMouseEvent>
#include <QApplication>
#include <QMenu>
#include <QHeaderView>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QPainterStateGuard>
#include <QStyle>
#include <QTextLayout>

namespace inviwo {

class ProcessorItemDelegate : public QStyledItemDelegate {
    using QStyledItemDelegate::QStyledItemDelegate;
    using Role = ProcessorListModel::Role;
    using Item = ProcessorListModel::Item;

    virtual QSize sizeHint(const QStyleOptionViewItem& option,
                           const QModelIndex& index) const override {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        // size.setHeight(28);
        return size;
    }

    // Reimplementation of the paint method to draw inports and outports as colored rectangles on
    // the processor items in the list view. See private impl at
    // https://codebrowser.dev/qt6/qtbase/src/widgets/styles/qcommonstyle.cpp.html#2271
    virtual void paint(QPainter* p, const QStyleOptionViewItem& option,
                       const QModelIndex& index) const override {

        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        const auto* widget = opt.widget;
        auto* style = widget->style();

        static constexpr auto portSize = QSize{6, 6};
        static constexpr auto padding = 2;
        static constexpr auto borderColor = QColor{30, 30, 30};
        static constexpr auto step = -QPoint{padding + portSize.width(), 0};
        const auto inportOffset =
            opt.rect.topRight() + QPoint{-padding - portSize.height(), padding};
        const auto outportOffset = opt.rect.bottomRight() + QPoint{-padding - portSize.height(),
                                                                   -padding - portSize.height()};

        const QPainterStateGuard psg(p);
        // the style calling this might want to clip, so respect any region already set
        const QRegion clipRegion = p->hasClipping() ? (p->clipRegion() & opt.rect) : opt.rect;
        p->setClipRegion(clipRegion);

        auto textRect = opt.rect.adjusted(32, 2, -2, -2);
        auto iconRect = opt.rect.adjusted(2, 2, -2, -2);
        iconRect.setWidth(28);

        // draw the background
        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, p, widget);

        if (const auto* item = utilqt::getData(index, Role::Item).value<const Item*>()) {
            const QPainterStateGuard psg2(p);

            switch (item->info.codeState) {
                case CodeState::Stable:
                    p->setBrush(QColor(38, 86, 115));
                    break;
                case CodeState::Experimental:
                    p->setBrush(QColor(141, 92, 21));
                    break;
                case CodeState::Deprecated:
                    p->setBrush(QColor(77, 77, 80));
                    break;
                case CodeState::Broken:
                    p->setBrush(QColor(115, 107, 39));
                    break;
            }
            p->setPen(QPen(borderColor, 1.0f));
            p->drawRoundedRect(iconRect, 8.0, 8.0);

            auto platformTags = util::getPlatformTags(item->info.tags);
            auto tag = platformTags.tags_.empty() ? Tags::CPU : platformTags.tags_.front();

            QPalette::ColorGroup cg =
                opt.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
            if (cg == QPalette::Normal && !(opt.state & QStyle::State_Active)) {
                cg = QPalette::Inactive;
            }
            // p->setPen(opt.palette.color(cg, (opt.state & QStyle::State_Selected)
            //                                     ? QPalette::HighlightedText
            //                                     : QPalette::Text));
            p->setPen(opt.palette.color(cg, QPalette::HighlightedText));
            // Make the painter draw a bold font
            p->setFont(QFont(opt.font.family(), opt.font.pointSize() - 4, QFont::Bold));
            p->drawText(iconRect, Qt::AlignCenter, utilqt::toQString(tag.getString()));

            p->setPen(QPen(borderColor, 1.0f));
            for (auto&& [i, port] :
                 util::enumerate<int>(item->help.inports | std::views::reverse)) {
                p->setBrush(utilqt::toQColor(port.colorCode));
                p->drawRect(QRect(inportOffset + i * step, portSize));
            }

            for (auto&& [i, port] :
                 util::enumerate<int>(item->help.outports | std::views::reverse)) {
                p->setBrush(utilqt::toQColor(port.colorCode));
                p->drawRect(QRect(outportOffset + i * step, portSize));
            }

        } else {
            textRect = opt.rect.adjusted(padding, padding, -padding, -padding);
        }

        // draw the text
        if (!opt.text.isEmpty()) {
            QPalette::ColorGroup cg =
                opt.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
            if (cg == QPalette::Normal && !(opt.state & QStyle::State_Active)) {
                cg = QPalette::Inactive;
            }
            p->setPen(opt.palette.color(cg, (opt.state & QStyle::State_Selected)
                                                ? QPalette::HighlightedText
                                                : QPalette::Text));

            // p->setFont(QFont(opt.font.family(), opt.font.pointSize() + 2, QFont::Bold));

            const int textMargin =
                style->pixelMetric(QStyle::PM_FocusFrameHMargin, nullptr, widget) + 1;

            // remove width padding
            const QRect textMarginRect = textRect.adjusted(textMargin, 0, -textMargin, 0);

            const auto elided =
                opt.fontMetrics.elidedText(opt.text, opt.textElideMode, textMarginRect.width());
            p->drawText(textMarginRect, static_cast<int>(opt.displayAlignment), elided);
        }

        // draw the focus rect
        if (opt.state & QStyle::State_HasFocus) {
            QStyleOptionFocusRect o;
            o.QStyleOption::operator=(opt);
            o.rect = style->subElementRect(QStyle::SE_ItemViewItemFocusRect, &opt, widget);
            o.state |= QStyle::State_KeyboardFocusChange;
            o.state |= QStyle::State_Item;
            const QPalette::ColorGroup cg =
                (opt.state & QStyle::State_Enabled) ? QPalette::Normal : QPalette::Disabled;
            o.backgroundColor = opt.palette.color(
                cg, (opt.state & QStyle::State_Selected) ? QPalette::Highlight : QPalette::Window);
            style->drawPrimitive(QStyle::PE_FrameFocusRect, &o, p, widget);
        }
    }
};

ProcessorListView::ProcessorListView(QAbstractItemModel* model, ProcessorListWidget* parent)
    : QTreeView(parent), processorTreeWidget_{parent} {
    setContextMenuPolicy(Qt::CustomContextMenu);

    setModel(model);

    setDragDropMode(QAbstractItemView::DragOnly);

    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);

    setHeaderHidden(true);
    setIndentation(utilqt::emToPx(this, 0.6));
    setAnimated(true);
    header()->setStretchLastSection(false);
    header()->setSectionResizeMode(0, QHeaderView::Stretch);
    header()->setDefaultSectionSize(utilqt::emToPx(this, 4.0));
    expandAll();
    setUniformRowHeights(true);

    setItemDelegate(new ProcessorItemDelegate(this));

    QObject::connect(this, &QTreeView::customContextMenuRequested, this,
                     &ProcessorListView::showContextMenu);
}

void ProcessorListView::mousePressEvent(QMouseEvent* e) {
    if (e->buttons() & Qt::LeftButton) dragStartPosition_ = e->pos();

    QTreeView::mousePressEvent(e);
}

void ProcessorListView::mouseMoveEvent(QMouseEvent* e) {
    if ((e->buttons() & Qt::LeftButton) && dragStartPosition_) {
        if ((e->pos() - *dragStartPosition_).manhattanLength() <
            QApplication::startDragDistance()) {
            return;
        }
        using Role = ProcessorListModel::Role;
        using Type = ProcessorListModel::Node::Type;

        auto index = indexAt(*dragStartPosition_);
        if (index.isValid() &&
            utilqt::getData(index, Role::Type).toInt() == static_cast<int>(Type::Item)) {
            dragStartPosition_.reset();
            auto id = utilqt::getData(index, Role::ClassIdentifier).toString();
            try {
                if (auto p = processorTreeWidget_->createProcessor(id)) {
                    auto* drag = new ProcessorDragObject(this, std::move(p));
                    drag->exec(Qt::MoveAction);
                }
            } catch (const std::exception& e) {
                log::error("Error trying to create processor: {} Message:\n{}",
                           utilqt::fromQString(id), e.what());
            }
        }
    }
}

void ProcessorListView::mouseReleaseEvent(QMouseEvent* e) {
    dragStartPosition_.reset();
    QTreeView::mouseReleaseEvent(e);
}

QModelIndex ProcessorListView::findFirstLeaf(QAbstractItemModel* model, QModelIndex parent) {
    if (!model->hasChildren(parent)) return parent;
    return findFirstLeaf(model, model->index(0, 0, parent));
}

void ProcessorListView::focusInEvent(QFocusEvent* e) {
    QTreeView::focusInEvent(e);

    if (e->reason() == Qt::MouseFocusReason) return;

    auto firstLeaf = findFirstLeaf(model());
    if (firstLeaf.isValid()) {
        selectionModel()->setCurrentIndex(firstLeaf, QItemSelectionModel::ClearAndSelect);
    }
}

void ProcessorListView::focusOutEvent(QFocusEvent* e) { QTreeView::focusOutEvent(e); }

void ProcessorListView::showContextMenu(const QPoint& p) {
    using Role = ProcessorListModel::Role;

    auto index = indexAt(p);
    auto id = utilqt::getData(index, Role::ClassIdentifier).toString();

    const bool enableExpandCollapse =
        ((processorTreeWidget_->getGrouping() != ProcessorListModel::Grouping::LastUsed) &&
         (processorTreeWidget_->getGrouping() != ProcessorListModel::Grouping::MostUsed));

    QMenu menu(this);
    auto* addItem = menu.addAction("&Add Processor");
    addItem->setEnabled(!id.isEmpty());
    QObject::connect(addItem, &QAction::triggered, this,
                     [&, id]() { processorTreeWidget_->addProcessor(id); });

    menu.addSeparator();

    auto* expandAction = menu.addAction("&Expand All Categories");
    expandAction->setEnabled(enableExpandCollapse);
    QObject::connect(expandAction, &QAction::triggered, this, [&]() { expandAll(); });

    auto* collapseOtherAction = menu.addAction("C&ollapse Others");
    collapseOtherAction->setEnabled(index.isValid() && enableExpandCollapse);
    // figure out top-level tree item, i.e. category
    while (index.isValid() && index.parent().isValid()) {
        index = index.parent();
    }
    QObject::connect(collapseOtherAction, &QAction::triggered, this, [&, index]() {
        collapseAll();
        expand(index);
    });

    auto* collapse = menu.addAction("&Collapse All Categories");
    collapse->setEnabled(enableExpandCollapse);
    QObject::connect(collapse, &QAction::triggered, this, [&]() { collapseAll(); });

    menu.exec(mapToGlobal(p));
}

ProcessorDragObject::ProcessorDragObject(QWidget* source, std::shared_ptr<Processor> processor)
    : QDrag(source) {
    auto img = QPixmap::fromImage(utilqt::generateProcessorPreview(*processor, 1.0));
    setPixmap(img);
    auto* mime = new ProcessorMimeData(std::move(processor));
    setMimeData(mime);
    setHotSpot(QPoint(img.width() / 2, img.height() / 2));
}

}  // namespace inviwo
