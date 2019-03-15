/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#ifndef IVW_LABELGRAPHICSITEM_H
#define IVW_LABELGRAPHICSITEM_H

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <inviwo/core/util/observer.h>
#include <warn/push>
#include <warn/ignore/all>
#include <QGraphicsTextItem>
#include <warn/pop>

namespace inviwo {
class LabelGraphicsItem;

class IVW_MODULE_QTWIDGETS_API LabelGraphicsItemObserver : public Observer {
public:
    LabelGraphicsItemObserver() = default;
    virtual ~LabelGraphicsItemObserver() = default;

    /**
     * This method will be called when observed object changes.
     * Override it to add behavior.
     */
    virtual void onLabelGraphicsItemChanged(LabelGraphicsItem*){};

    virtual void onLabelGraphicsItemEdited(LabelGraphicsItem*){};
};
class IVW_MODULE_QTWIDGETS_API LabelGraphicsItemObservable
    : public Observable<LabelGraphicsItemObserver> {
public:
    LabelGraphicsItemObservable() = default;
    virtual ~LabelGraphicsItemObservable() = default;
    void notifyObserversChanged(LabelGraphicsItem*);
    void notifyObserversEdited(LabelGraphicsItem*);
};

class IVW_MODULE_QTWIDGETS_API LabelGraphicsItem : public QGraphicsTextItem,
                                                   public LabelGraphicsItemObservable {

public:
    LabelGraphicsItem(QGraphicsItem* parent, int width = 30,
                      Qt::Alignment alignment = Qt::AlignLeft | Qt::AlignTop);
    virtual ~LabelGraphicsItem() = default;

    QString text() const;
    void setText(const QString&);
    void setHtml(const QString& str);

    QString croppedText() const;
    void setCrop(int width);
    bool isCropped() const;

    int usedTextWidth() const;

    void setNoFocusOut();
    bool isFocusOut() const;

    void setAlignment(Qt::Alignment alignment);

protected:
    virtual void keyPressEvent(QKeyEvent* keyEvent) override;
    virtual void focusInEvent(QFocusEvent* event) override;
    virtual void focusOutEvent(QFocusEvent* event) override;

    void updatePosition();

private:
    int width_;
    bool focusOut_;
    QString orgText_;
    Qt::Alignment alignment_;  // Qt::AlignLeft/Right/HCenter | Qt::AlignTop/Bottom/VCenter
};

}  // namespace inviwo

#endif  // IVW_LABELGRAPHICSITEM_H
