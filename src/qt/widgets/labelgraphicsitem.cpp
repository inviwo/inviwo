/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#include <warn/push>
#include <warn/ignore/all>
#include <QFocusEvent>
#include <QFont>
#include <QPainter>
#include <QTextCursor>
#include <warn/pop>

#include <inviwo/qt/widgets/labelgraphicsitem.h>

namespace inviwo {

LabelGraphicsItem::LabelGraphicsItem(QGraphicsItem* parent)
    : QGraphicsTextItem(parent), LabelGraphicsItemObservable()
    , maxBefore_(0)
    , maxAfter_(0)
    , focusOut_(false)
    , orgText_("")
{
    font().setPixelSize(4);
}

LabelGraphicsItem::~LabelGraphicsItem() {}

QString LabelGraphicsItem::text() const {
    if (isCropped())
        return orgText_;
    else
        return toPlainText();
}

void LabelGraphicsItem::setText(const QString& str) {
    if (doCrop(str)) {
        if (toolTip()==orgText_)
            setToolTip(str);

        orgText_ = str;
        setPlainText(str.left(maxBefore_) + "..." + str.right(maxAfter_));
    }
    else {
        orgText_="";
        setToolTip(orgText_);
        setPlainText(str);
    }
}

QString LabelGraphicsItem::croppedText() const {
    return toPlainText();
}

void LabelGraphicsItem::setCrop(int before, int after) {
    maxBefore_ = before;
    maxAfter_ = after;
    updateCrop();
}

bool LabelGraphicsItem::isCropped() const {
    return (!orgText_.isEmpty());
}

void LabelGraphicsItem::setNoFocusOut() {
    focusOut_ = false;
}

bool LabelGraphicsItem::isFocusOut() const {
    return focusOut_;
}

bool LabelGraphicsItem::doCrop(const QString& str) {
    return (maxBefore_ + maxAfter_ + 2 < str.length());
}

void LabelGraphicsItem::updateCrop() {
    setText(toPlainText());
}

void LabelGraphicsItem::keyPressEvent(QKeyEvent* keyEvent) {
    if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
        clearFocus();
    } else {
        QGraphicsTextItem::keyPressEvent(keyEvent);
        keyEvent->accept();
    }
}

void LabelGraphicsItem::focusInEvent(QFocusEvent* event) {
    if (isCropped())
        setPlainText(orgText_);
}

void LabelGraphicsItem::focusOutEvent(QFocusEvent* event) {
    focusOut_ = true;
    setFlags(nullptr);
    setTextInteractionFlags(Qt::NoTextInteraction);
    QTextCursor cur = QTextCursor(textCursor());
    cur.clearSelection();
    setTextCursor(cur);
    updateCrop();
    notifyLabelGraphicsItemObservers();
    focusOut_ = false;
}

} // namespace
