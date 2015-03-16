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

#ifndef IVW_LABELGRAPHICSITEM_H
#define IVW_LABELGRAPHICSITEM_H

#include <inviwo/qt/widgets/inviwoqtwidgetsdefine.h>
#include <QGraphicsTextItem>
#include <inviwo/core/util/observer.h>

namespace inviwo {

class IVW_QTWIDGETS_API LabelGraphicsItemObserver: public Observer {
public:
    LabelGraphicsItemObserver(): Observer() {};

    /**
    * This method will be called when observed object changes.
    * Override it to add behavior.
    */
    virtual void onLabelGraphicsItemChange() {};
};
class IVW_QTWIDGETS_API LabelGraphicsItemObservable: public Observable<LabelGraphicsItemObserver> {
public:
    LabelGraphicsItemObservable(): Observable<LabelGraphicsItemObserver>() {};

    void notifyLabelGraphicsItemObservers() const {
        // Notify observers
        for (ObserverSet::reverse_iterator it = observers_->rbegin(); it != observers_->rend(); ++it) {
            // static_cast can be used since only template class objects can be added
            static_cast<LabelGraphicsItemObserver*>(*it)->onLabelGraphicsItemChange();
        }
    }
};

class IVW_QTWIDGETS_API LabelGraphicsItem : public QGraphicsTextItem, public LabelGraphicsItemObservable {

public:
    LabelGraphicsItem(QGraphicsItem* parent);
    ~LabelGraphicsItem();

    QString text() const;
    void setText(const QString&);

    QString croppedText() const;
    void setCrop(int, int);
    bool isCropped() const;

    void setNoFocusOut();
    bool isFocusOut() const;

protected:
    bool doCrop(const QString& str);
    void updateCrop();

    void keyPressEvent(QKeyEvent* keyEvent);
    void focusInEvent(QFocusEvent* event);
    void focusOutEvent(QFocusEvent* event);

private:
    int maxBefore_;
    int maxAfter_;
    bool focusOut_;
    QString orgText_;
};

} // namespace

#endif // IVW_LABELGRAPHICSITEM_H