/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2024 Inviwo Foundation
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

#include <modules/qtwidgets/labelgraphicsitem.h>  // for LabelGraphicsItem, LabelGraphicsItemObs...

#include <vector>  // for vector

#include <QFlags>             // for QFlags
#include <QFont>              // for QFont
#include <QFontMetrics>       // for QFontMetrics
#include <QGraphicsItem>      // for QGraphicsItem::GraphicsItemFlags
#include <QGraphicsTextItem>  // for QGraphicsTextItem
#include <QKeyEvent>          // for QKeyEvent
#include <QRect>              // for QRect
#include <QRectF>             // for QRectF
#include <QString>            // for QString, operator!=
#include <QTextCursor>        // for QTextCursor
#include <QTextDocument>      // for QTextDocument
#include <QTransform>         // for QTransform
#include <Qt>                 // for Alignment, AlignBottom, AlignHCenter

class QFocusEvent;
class QKeyEvent;

namespace inviwo {

LabelGraphicsItem::LabelGraphicsItem(QGraphicsItem* parent, int width, Qt::Alignment alignment)
    : QGraphicsTextItem(parent)
    , LabelGraphicsItemObservable()
    , width_(width)
    , focusOut_(false)
    , orgText_("")
    , alignment_(alignment) {
    font().setPixelSize(4);
    document()->setDocumentMargin(1.0);
}

QString LabelGraphicsItem::text() const {
    if (isCropped())
        return orgText_;
    else
        return toPlainText();
}

void LabelGraphicsItem::setText(const QString& str) {
    QFontMetrics fm = QFontMetrics(font());
    auto text = fm.elidedText(str, Qt::ElideMiddle, width_);
    setPlainText(text);
    setToolTip(str);
    orgText_ = str;

    updatePosition();
}

void LabelGraphicsItem::setHtml(const QString& str) {
    QGraphicsTextItem::setHtml(str);
    updatePosition();
}

void LabelGraphicsItem::updatePosition() {
    // adjust transformation
    QRectF b = QGraphicsTextItem::boundingRect();
    double x = b.x();
    double y = b.y();
    // horizontal alignment
    if (alignment_ & Qt::AlignHCenter) {
        x -= b.width() / 2.0;
    } else if (alignment_ & Qt::AlignRight) {
        x -= b.width();
    }
    // vertical alignment
    if (alignment_ & Qt::AlignVCenter) {
        y -= b.height() / 2.0;
    } else if (alignment_ & Qt::AlignBottom) {
        y -= b.height();
    }

    QTransform t(QTransform::fromTranslate(x, y));
    setTransform(t);
}

QString LabelGraphicsItem::croppedText() const { return toPlainText(); }

void LabelGraphicsItem::setCrop(int width) {
    width_ = width;
    setText(orgText_);
}

bool LabelGraphicsItem::isCropped() const { return (orgText_ != toPlainText()); }

int LabelGraphicsItem::usedTextWidth() const {
    QFontMetrics fm = QFontMetrics(font());
    return fm.tightBoundingRect(croppedText()).width();
}

void LabelGraphicsItem::setNoFocusOut() { focusOut_ = false; }

bool LabelGraphicsItem::isFocusOut() const { return focusOut_; }

void LabelGraphicsItem::setAlignment(Qt::Alignment alignment) {
    QGraphicsTextItem::prepareGeometryChange();
    alignment_ = alignment;
    updatePosition();
}

void LabelGraphicsItem::keyPressEvent(QKeyEvent* keyEvent) {
    if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
        clearFocus();
    } else {
        QGraphicsTextItem::keyPressEvent(keyEvent);
        keyEvent->accept();
    }
    notifyObserversEdited(this);
}

void LabelGraphicsItem::focusInEvent(QFocusEvent*) {
    if (isCropped()) setPlainText(orgText_);
}

void LabelGraphicsItem::focusOutEvent(QFocusEvent*) {
    focusOut_ = true;
    setFlags(GraphicsItemFlags{});
    setTextInteractionFlags(Qt::NoTextInteraction);
    QTextCursor cur = QTextCursor(textCursor());
    cur.clearSelection();
    setTextCursor(cur);
    setText(toPlainText());
    notifyObserversChanged(this);
    focusOut_ = false;
}

void LabelGraphicsItemObservable::notifyObserversChanged(LabelGraphicsItem* item) {
    forEachObserver([&](LabelGraphicsItemObserver* o) { o->onLabelGraphicsItemChanged(item); });
}

void LabelGraphicsItemObservable::notifyObserversEdited(LabelGraphicsItem* item) {
    forEachObserver([&](LabelGraphicsItemObserver* o) { o->onLabelGraphicsItemEdited(item); });
}

}  // namespace inviwo
