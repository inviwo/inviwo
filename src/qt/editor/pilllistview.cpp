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

#include <inviwo/qt/editor/pilllistview.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <QStyledItemDelegate>
#include <QPainter>

namespace inviwo {

TagModel::TagModel(Tags tags, QObject* parent) : QAbstractListModel(parent) {
    for (const auto& t : tags.tags_) {
        tags_.push_back({t, utilqt::toQString(t.getString()), true});
    }
}

int TagModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : static_cast<int>(tags_.size());
}

QVariant TagModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return {};

    const Item& tag = tags_[index.row()];

    switch (role) {
        case Qt::DisplayRole:
            return tag.text;
        case Qt::CheckStateRole:
            return tag.checked ? Qt::Checked : Qt::Unchecked;
        default:
            return {};
    }
}

Tags TagModel::checkedTags() const {
    Tags checked;
    for (const auto& tag : tags_) {
        if (tag.checked) checked.addTag(tag.tag);
    }
    return checked;
}

bool TagModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid()) return false;

    Item& tag = tags_[index.row()];
    if (role == Qt::CheckStateRole) {
        tag.checked = (value.toInt() == Qt::Checked);
        emit dataChanged(index, index, {Qt::CheckStateRole});
        return true;
    }

    return false;
}

Qt::ItemFlags TagModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) return Qt::NoItemFlags;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
}

class PillDelegate : public QStyledItemDelegate {
public:
    explicit PillDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

    static constexpr int hPad = 2;
    static constexpr int vPad = 2;
    static constexpr int radius = 8;

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        const bool checked = index.data(Qt::CheckStateRole) == Qt::Checked;
        const bool hovered = option.state & QStyle::State_MouseOver;

        QColor bg = checked ? QColor(38, 86, 115) : QColor("#4f555b");
        QColor fg = checked || hovered ? Qt::white : QColor("#9d9995");

        QRect r = option.rect.adjusted(2, 2, -2, -2);

        painter->setBrush(bg);
        painter->setPen(QPen{hovered ? QColor(7, 55, 84) : QColor(30, 30, 30), 1.0f});
        painter->drawRoundedRect(r, radius, radius);

        painter->setPen(fg);
        painter->drawText(r.adjusted(hPad, vPad, -hPad, -vPad), Qt::AlignCenter,
                          index.data().toString());

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        const QString text = index.data().toString();
        QSize textSize = option.fontMetrics.size(Qt::TextSingleLine, text);
        return QSize(textSize.width() + hPad * 2 + radius,
                     textSize.height() + vPad * 2 + radius / 2);
    }
};

PillListView::PillListView(QWidget* parent) : QListView(parent) {
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    setItemDelegate(new PillDelegate(this));

    setFlow(QListView::LeftToRight);
    setWrapping(true);
    setResizeMode(QListView::Adjust);
    setSelectionMode(QAbstractItemView::NoSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFocusPolicy(Qt::StrongFocus);
    setSpacing(0);
    QFont f = font();
    f.setPointSizeF(f.pointSizeF() * 0.8);
    setFont(f);
}

int PillListView::heightForWidth(int width) const {
    if (!model() || model()->rowCount() == 0) return 0;

    // Ask Qt to compute layout for this width
    const_cast<PillListView*>(this)->doItemsLayout();

    QModelIndex last = model()->index(model()->rowCount() - 1, 0);
    QRect r = visualRect(last);

    return r.bottom() + frameWidth() * 2 + spacing();
}

QSize PillListView::viewportSizeHint() const {
    // Ensures wrapping reacts to width changes
    return QSize(viewport()->width(), heightForWidth(width()));
}

}  // namespace inviwo
